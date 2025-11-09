#include "state_box2d.hpp"

#include "color/color_factory.hpp"
#include "contact_callback_player_enemy.hpp"
#include "contact_callback_player_ground.hpp"
#include "conversions.hpp"
#include "drawable_helpers.hpp"
#include "game_properties.hpp"
#include "lerp.hpp"
#include "state_menu.hpp"
#include "tweens/tween_scale.hpp"
#include <box2dwrapper/box2d_contact_manager.hpp>
#include <box2dwrapper/box2d_world_impl.hpp>
#include <box2dwrapper/logging_box2d_contact_manager.hpp>
#include <game_interface.hpp>
#include <input/input_manager.hpp>
#include <platform_player.hpp>
#include <random/random.hpp>
#include <tweens/tween_alpha.hpp>
#include <tweens/tween_position.hpp>
#include <tweens/tween_rotation.hpp>

#include "SpiderString.h"

StatePlatformer::StatePlatformer(PlayerType pt, std::string const& levelName)
{
    m_playerType = pt;
    m_levelName = levelName;
}

class FancyCallbackFixtureThingy : public b2RayCastCallback {
public:
    FancyCallbackFixtureThingy(b2Fixture* playerFixture)
    {
        hitSomething = false;
        m_playerFixture = playerFixture;
        m_distanceToBestHit = std::numeric_limits<float>::max();
    }

    float32 ReportFixture(
        b2Fixture* fixture, b2Vec2 const& point, b2Vec2 const& normal, float32 fraction) override
    {
        if (fixture == m_playerFixture) {
            return -1;
        }

        hitSomething = true;
        if (fraction < m_distanceToBestHit) {
            m_distanceToBestHit = fraction;
            hitPoint = point;
        }

        return fraction;
    }

    b2Vec2 hitPoint;
    bool hitSomething;

private:
    b2Fixture* m_playerFixture;
    float m_distanceToBestHit;
};

std::shared_ptr<jt::ParticleSystem<jt::Shape, 100>> StatePlatformer::createStringParticles(
    int index)
{
    auto color = getStringColor(index);
    auto particles = jt::ParticleSystem<jt::Shape, 100>::createPS(
        [this, color]() {
            auto s = jt::dh::createShapeCircle(2, color, textureManager());
            s->setOrigin(jt::Vector2f { 0, 0 });
            s->setScale(jt::Vector2f { 0.5f, 0.5f });
            return s;
        },
        [this](auto& s, auto pos) {
            auto totalTime = 1;
            auto const ageFn = [](float t) {
                t = std::clamp(t, 0.0f, 1.0f);
                return t > 0.999 ? 1.0f : 1.0f - pow(2.0f, -5.0f * t);
            };

            s->setPosition(jt::Random::getRandomPointIn(jt::Rectf { 0, 0, 100, 300 }));

            auto twa = jt::TweenAlpha::create(s, totalTime, 255, 0);
            twa->setAgePercentConversion(ageFn);
            add(twa);

            auto tws = jt::TweenScale::create(
                s, totalTime, jt::Vector2f { 1.0, 1.0 }, jt::Vector2f { 0, 0 });
            tws->setAgePercentConversion(ageFn);
            add(tws);

            auto const maxVariance = 20.0f;
            auto const startPos = pos;
            auto const endPos = pos + jt::Random::getRandomPointInCircle(maxVariance);
            auto twp = jt::TweenPosition::create(s, totalTime, startPos, endPos);
            twp->setAgePercentConversion(ageFn);
            add(twp);
        });
    add(particles);
    return particles;
}

void StatePlatformer::onCreate()
{
    auto contactManager = std::make_shared<jt::Box2DContactManager>();
    auto loggingContactManager
        = std::make_shared<jt::LoggingBox2DContactManager>(contactManager, getGame()->logger());
    m_world = std::make_shared<jt::Box2DWorldImpl>(
        jt::Vector2f { 0.0f, GP::PhysicsGravityStrength() }, loggingContactManager);

    m_background = std::make_shared<jt::Shape>();
    m_background->makeRect(GP::GetScreenSize(), textureManager());
    m_background->setColor(jt::ColorFactory::fromHexString("#203835"));
    m_background->setCamMovementFactor(0.0f);
    loadLevel();

    CreatePlayer();
    getGame()->gfx().camera().setCamOffset(m_player->getPosition() - 0.5f * GP::GetScreenSize());

    auto playerGroundContactListener = std::make_shared<ContactCallbackPlayerGround>();
    playerGroundContactListener->setPlayer(m_player);
    m_world->getContactManager().registerCallback("player_ground", playerGroundContactListener);

    auto playerEnemyContactListener = std::make_shared<ContactCallbackPlayerEnemy>();
    playerEnemyContactListener->setPlayer(m_player);
    m_world->getContactManager().registerCallback("player_enemy", playerEnemyContactListener);

    m_vignette = std::make_shared<jt::Vignette>(GP::GetScreenSize());
    add(m_vignette);

    m_scanlines = std::make_shared<jt::ScanLines>(
        jt::Vector2f { GP::GetScreenSize().x, GP::GetScreenSize().y / 256 }, 256);
    // add(m_scanlines);

    setAutoDraw(false);

    auto rng = std::default_random_engine {};
    for (int i = 0; i < m_stringParticleSystems.size(); i++) {
        m_stringParticleSystems[i] = createStringParticles(i);
    }
}

void StatePlatformer::onEnter() { }

void StatePlatformer::loadLevel()
{
    m_level = std::make_shared<Level>("assets/" + m_levelName, m_world);
    add(m_level);
}

void StatePlatformer::onUpdate(float const elapsed)
{
    m_background->update(elapsed);
    for (auto m_active_string : m_activeStrings) {
        if (m_active_string != nullptr && m_active_string->isAlive()) {
            m_active_string->update(elapsed);
        }
    }
    if (getGame()->input().keyboard()->justPressed(jt::KeyCode::Space)) {
        shootString(0, { 0.0, -1.0 });
    }

    if (!m_ending && !getGame()->stateManager().getTransition()->isInProgress()) {
        std::int32_t const velocityIterations = 20;
        std::int32_t const positionIterations = 20;
        m_world->step(elapsed, velocityIterations, positionIterations);

        if (!m_player->isAlive()) {
            endGame();
        }
        m_level->checkIfPlayerIsInKillbox(m_player->getPosition(), [this]() { endGame(); });
        m_level->checkIfPlayerIsInExit(
            m_player->getPosition(), [this](std::string const& newLevelName) {
                if (!m_ending) {
                    m_ending = true;
                    auto snd = getGame()->audio().addTemporarySound("event:/win-sound");
                    snd->play();
                    getGame()->stateManager().switchState(
                        std::make_shared<StatePlatformer>(m_playerType, newLevelName));
                }
            });

        handleCameraScrolling(elapsed);
    }
    if (getGame()->input().keyboard()->justPressed(jt::KeyCode::F1)
        || getGame()->input().keyboard()->justPressed(jt::KeyCode::Escape)) {

        getGame()->stateManager().switchState(std::make_shared<StateMenu>());
    }
}

void StatePlatformer::endGame()
{
    if (!m_ending) {
        m_ending = true;
        getGame()->stateManager().switchState(
            std::make_shared<StatePlatformer>(m_playerType, m_levelName));
    }
}

void StatePlatformer::handleCameraScrolling(float const elapsed)
{
    auto ps = m_player->getPosOnScreen();

    float const topMargin = 150.0f;
    float const bottomMargin = 50.0f;
    float const rightMargin = 150.0f;
    float const leftMargin = 150.0f;
    float const scrollSpeed = 60.0f;
    auto& cam = getGame()->gfx().camera();

    auto const screenWidth = GP::GetScreenSize().x;
    auto const screenHeight = GP::GetScreenSize().y;

    if (ps.x < leftMargin) {
        cam.move(jt::Vector2f { -scrollSpeed * elapsed, 0.0f });
        if (ps.x < rightMargin / 2) {
            cam.move(jt::Vector2f { -scrollSpeed * elapsed, 0.0f });
            if (ps.x < 0.0) {
                cam.move(jt::Vector2f { -scrollSpeed * elapsed, 0.0f });
            }
        }
    } else if (ps.x > screenWidth - rightMargin) {
        cam.move(jt::Vector2f { scrollSpeed * elapsed, 0.0f });
        if (ps.x > screenWidth - rightMargin / 3 * 2) {
            cam.move(jt::Vector2f { scrollSpeed * elapsed, 0.0f });
            if (ps.x > screenWidth) {
                cam.move(jt::Vector2f { scrollSpeed * elapsed, 0.0f });
            }
        }
    }
    if (ps.y < topMargin) {
        cam.move(jt::Vector2f { 0.0f, -scrollSpeed * elapsed });
        if (ps.y < bottomMargin / 2) {
            cam.move(jt::Vector2f { 0.0f, -scrollSpeed * elapsed });
            if (ps.y < 0.0) {
                cam.move(jt::Vector2f { 0.0f, -scrollSpeed * elapsed });
            }
        }
    } else if (ps.y > screenHeight - bottomMargin) {
        cam.move(jt::Vector2f { 0.0f, scrollSpeed * elapsed });
        if (ps.y > screenHeight - rightMargin / 3 * 2) {
            cam.move(jt::Vector2f { 0.0f, scrollSpeed * elapsed });
            if (ps.y > screenHeight) {
                cam.move(jt::Vector2f { 0.0f, scrollSpeed * elapsed });
            }
        }
    }

    // clamp camera to level bounds
    auto offset = cam.getCamOffset();
    if (offset.x < 0) {
        offset.x = 0;
    }
    if (offset.y < 0) {
        offset.y = 0;
    }
    auto const levelWidth = m_level->getLevelSizeInPixel().x;
    auto const levelHeight = m_level->getLevelSizeInPixel().y;
    auto const maxCamPositionX = levelWidth - screenWidth;
    auto const maxCamPositionY = levelHeight - screenHeight;
    // std::cout << ps.x << " " << screenWidth << " " << levelWidth << " " << maxCamPosition
    //           << std::endl;
    if (offset.x > maxCamPositionX) {
        offset.x = maxCamPositionX;
    }
    if (offset.y > maxCamPositionY) {
        offset.y = maxCamPositionY;
    }

    cam.setCamOffset(offset);
}

void StatePlatformer::onDraw() const
{
    m_background->draw(renderTarget());
    m_player->drawRopeTarget(renderTarget());
    for (auto particle_system : m_stringParticleSystems) {
        particle_system->draw();
    }
    m_level->draw();

    m_player->draw();
    m_walkParticles->draw();
    m_playerJumpParticles->draw();
    // m_scanlines->draw();
    m_vignette->draw();

    for (auto m_active_string : m_activeStrings) {
        if (m_active_string != nullptr && m_active_string->isAlive()) {
            m_active_string->draw();
        }
    }
}

void StatePlatformer::CreatePlayer()
{
    m_player = std::make_shared<Player>(m_playerType, m_world);
    m_player->setPosition(m_level->getPlayerStart());
    m_player->setLevelSize(m_level->getLevelSizeInPixel());
    add(m_player);

    m_player->setStringFireCallback(
        [this](int const index, jt::Vector2f const& dir) { shootString(index, dir); });

    createPlayerWalkParticles();
    createPlayerJumpParticleSystem();
}

void StatePlatformer::createPlayerJumpParticleSystem()
{
    m_playerJumpParticles = jt::ParticleSystem<jt::Shape, 50>::createPS(
        [this]() {
            auto s = std::make_shared<jt::Shape>();
            if (jt::Random::getChance()) {
                s->makeRect(jt::Vector2f { 1.0f, 1.0f }, textureManager());
            } else {
                s->makeRect(jt::Vector2f { 2.0f, 2.0f }, textureManager());
            }
            s->setColor(jt::colors::White);
            s->setPosition(jt::Vector2f { -50000, -50000 });
            s->setOrigin(jt::Vector2f { 1.0f, 1.0f });
            return s;
        },
        [this](auto s, auto p) {
            s->setPosition(p);
            s->update(0.0f);
            auto const totalTime = jt::Random::getFloat(0.2f, 0.3f);

            auto twa = jt::TweenAlpha::create(s, totalTime / 2.0f, 255, 0);
            twa->setStartDelay(totalTime / 2.0f);
            add(twa);

            auto const startPos = p;
            auto const endPos = p
                + jt::Vector2f { jt::Random::getFloatGauss(0, 4.5f),
                      jt::Random::getFloat(-2.0f, 0.0f) };
            auto twp = jt::TweenPosition::create(s, totalTime, startPos, endPos);
            add(twp);

            float minAngle = 0.0f;
            float maxAngle = 360.0f;
            if (endPos.x < startPos.x) {
                minAngle = 360.0f;
                maxAngle = 0.0f;
            }
            auto twr = jt::TweenRotation::create(s, totalTime, minAngle, maxAngle);
            add(twr);
        });
    add(m_playerJumpParticles);
    m_player->setJumpParticleSystem(m_playerJumpParticles);
}

void StatePlatformer::createPlayerWalkParticles()
{
    m_walkParticles = jt::ParticleSystem<jt::Shape, 50>::createPS(
        [this]() {
            auto s = std::make_shared<jt::Shape>();
            s->makeRect(jt::Vector2f { 1.0f, 1.0f }, textureManager());
            s->setColor(jt::colors::Black);
            s->setPosition(jt::Vector2f { -50000, -50000 });
            return s;
        },
        [this](auto s, auto p) {
            s->setPosition(p);

            auto twa = jt::TweenAlpha::create(s, 1.5f, 255, 0);
            add(twa);

            auto const rp
                = p + jt::Vector2f { 0, 4 } + jt::Vector2f { jt::Random::getFloat(-4, 4), 0 };

            auto topPos = rp;
            auto botPos = rp;
            auto const maxHeight = jt::Random::getFloat(2.0f, 7.0f);
            auto const maxWidth = jt::Random::getFloat(2.0f, 6.0f);
            if (jt::Random::getChance()) {
                topPos = rp + jt::Vector2f { maxWidth / 2, -maxHeight };
                botPos = rp + jt::Vector2f { maxWidth, 0 };
            } else {
                topPos = rp + jt::Vector2f { -maxWidth / 2, -maxHeight };
                botPos = rp + jt::Vector2f { -maxWidth, 0 };
            }
            auto const totalTime = jt::Random::getFloat(0.3f, 0.6f);
            std::shared_ptr<jt::Tween> twp1
                = jt::TweenPosition::create(s, totalTime / 2.0f, rp, topPos);
            add(twp1);
            twp1->addCompleteCallback([this, topPos, botPos, s, totalTime]() {
                auto twp2 = jt::TweenPosition::create(s, totalTime / 2.0f, topPos, botPos);
                add(twp2);
            });
        });
    add(m_walkParticles);
    m_player->setWalkParticleSystem(m_walkParticles);
}

std::string StatePlatformer::getName() const { return "Box2D"; }

jt::Color StatePlatformer::getStringColor(int stringIndex)
{
    if (stringIndex == 0) {
        return jt::Color { 16, 185, 16, 255 };
    }
    if (stringIndex == 1) {
        return jt::Color { 230, 41, 55, 255 };
    }
    if (stringIndex == 2) {
        return jt::Color { 0, 122, 204, 255 };
    }
    if (stringIndex == 3) {
        return jt::Color { 255, 211, 0, 255 };
    }
}

void StatePlatformer::shootString(int stringIndex, jt::Vector2f direction)
{
    if (jt::MathHelper::lengthSquared(direction) < 0.001) {
        return;
    }
    // std::cout << direction.x << "|" << direction.y << std::endl;
    auto& existingString = m_activeStrings[stringIndex];

    if (existingString != nullptr && existingString->isAlive()) {
        existingString.reset();
        m_tempStringAnchors[stringIndex].reset();
    }

    auto cb = FancyCallbackFixtureThingy { &m_player->getB2Body()->GetFixtureList()[0] };
    m_world->getWorld()->RayCast(&cb, m_player->getB2Body()->GetPosition(),
        m_player->getB2Body()->GetPosition()
            + jt::Conversion::vec(GP::PhysicsStringMaxLengthInPx() * direction));

    if (!cb.hitSomething) {
        auto snd = getGame()->audio().addTemporarySound("event:/faden-trifft-nicht");
        snd->play();
        return;
    }

    b2BodyDef target;
    target.fixedRotation = true;
    target.position = cb.hitPoint;
    target.type = b2_kinematicBody;

    m_tempStringAnchors[stringIndex] = std::make_shared<jt::Box2DObject>(m_world, &target);

    existingString = std::make_shared<SpiderString>(
        m_world, m_player->getB2Body(), m_tempStringAnchors[stringIndex]->getB2Body());
    add(existingString);
    jt::Color const c = getStringColor(stringIndex);
    existingString->withTargetCircle();
    existingString->setStringColor(c);

    m_activeStrings[stringIndex] = existingString;
    m_stringParticleSystems[stringIndex]->fire(50, jt::Conversion::vec(cb.hitPoint));
    auto snd = getGame()->audio().addTemporarySound("event:/faden-abschießen");
    snd->play();
}
