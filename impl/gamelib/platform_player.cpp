#include "platform_player.hpp"

#include "game_properties.hpp"
#include <game_interface.hpp>
#include <math_helper.hpp>
#include <user_data_entries.hpp>

Player::Player(std::shared_ptr<jt::Box2DWorldInterface> world)
{
    b2BodyDef bodyDef;
    bodyDef.fixedRotation = true;
    bodyDef.type = b2_dynamicBody;

    m_physicsObject = std::make_shared<jt::Box2DObject>(world, &bodyDef);
}

void Player::doCreate()
{
    m_animation = std::make_shared<jt::Animation>();

    m_animation->loadFromAseprite("assets/arachno.aseprite", textureManager());
    m_animation->play("idle");
    m_animation->setOffset(jt::OffsetMode::CENTER);

    b2FixtureDef fixtureDef;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.5f;
    b2CircleShape circleCollider {};
    circleCollider.m_radius = 8.0f;
    fixtureDef.shape = &circleCollider;
    auto playerCollider = m_physicsObject->getB2Body()->CreateFixture(&fixtureDef);
    playerCollider->SetUserData((void*)g_userDataPlayerID);

    fixtureDef.isSensor = true;
    b2PolygonShape polygonShape;
    polygonShape.SetAsBox(3.0f, 0.2f, b2Vec2(0, 4), 0);
    fixtureDef.shape = &polygonShape;
    auto footSensorFixture = m_physicsObject->getB2Body()->CreateFixture(&fixtureDef);
    footSensorFixture->SetUserData((void*)g_userDataPlayerFeetID);

    m_crosshairH = std::make_shared<jt::Line>(jt::Vector2f { 16.0f, 0.0f });
    m_crosshairV = std::make_shared<jt::Line>(jt::Vector2f { 0.0f, 16.0f });

    m_targetLine = std::make_shared<jt::Line>(jt::Vector2f { 0.0f, 0.0f });
    m_targetLine->setColor(jt::Color { 255, 255, 255, 100 });
}

std::shared_ptr<jt::Animation> Player::getAnimation() { return m_animation; }

void Player::doUpdate(float const elapsed)
{
    auto currentPosition = m_physicsObject->getPosition();
    clampPositionToLevelSize(currentPosition);
    m_physicsObject->setPosition(currentPosition);
    m_animation->setPosition(currentPosition);
    updateAnimation(elapsed);
    handleMovement(elapsed);

    m_crosshairH->update(elapsed);
    m_crosshairV->update(elapsed);

    jt::Vector2f dir = m_gpAxis;
    jt::MathHelper::normalizeMe(dir);
    m_targetLine->setLineVector(GP::PhysicsStringMaxLengthInPx() * dir);
    m_targetLine->setPosition(getPosition());
    m_targetLine->update(elapsed);
}

void Player::clampPositionToLevelSize(jt::Vector2f& currentPosition) const
{
    if (currentPosition.x < 4) {
        currentPosition.x = 4;
    }
    if (currentPosition.x > m_levelSizeInTiles.x - 4) {
        currentPosition.x = m_levelSizeInTiles.x - 4;
    }
}

void Player::updateAnimation(float elapsed) { m_animation->update(elapsed); }

void Player::handleMovement(float const /*elapsed*/)
{
    auto gp = getGame()->input().gamepad(0).get();

    jt::Vector2f gpAxis = gp->getAxis(jt::GamepadAxisCode::ALeft);

    if (jt::MathHelper::lengthSquared(gpAxis) > 0.005f) {
        jt::MathHelper::normalizeMe(gpAxis);
        m_gpAxis = 48.0f * gpAxis;
    }
    m_crosshairPos = getPosition() + m_gpAxis;
    m_crosshairH->setPosition(m_crosshairPos - jt::Vector2f { 8.0f, 0.0f });
    m_crosshairV->setPosition(m_crosshairPos - jt::Vector2f { 0.0f, 8.0f });

    if (m_fireStringCallback) {
        if (gp->justPressed(jt::GamepadButtonCode::GBA)) {
            m_fireStringCallback(0, gpAxis);
        } else if (gp->justPressed(jt::GamepadButtonCode::GBB)) {
            m_fireStringCallback(1, gpAxis);
        } else if (gp->justPressed(jt::GamepadButtonCode::GBX)) {
            m_fireStringCallback(2, gpAxis);
        } else if (gp->justPressed(jt::GamepadButtonCode::GBY)) {
            m_fireStringCallback(3, gpAxis);
        }
    }
}

b2Body* Player::getB2Body() { return m_physicsObject->getB2Body(); }

void Player::doDraw() const
{
    m_animation->draw(renderTarget());
    m_crosshairH->draw(renderTarget());
    m_crosshairV->draw(renderTarget());
}

void Player::setTouchesGround(bool /*touchingGround*/) { }

jt::Vector2f Player::getPosOnScreen() const { return m_animation->getScreenPosition(); }

void Player::setPosition(jt::Vector2f const& pos) { m_physicsObject->setPosition(pos); }

jt::Vector2f Player::getPosition() const { return m_physicsObject->getPosition(); }

void Player::setWalkParticleSystem(std::weak_ptr<jt::ParticleSystem<jt::Shape, 50>> ps) { }

void Player::setJumpParticleSystem(std::weak_ptr<jt::ParticleSystem<jt::Shape, 50>> ps) { }

void Player::setLevelSize(jt::Vector2f const& levelSizeInTiles)
{
    m_levelSizeInTiles = levelSizeInTiles;
}

void Player::setStringFireCallback(std::function<void(int, jt::Vector2f const&)> const& callback)
{
    m_fireStringCallback = callback;
}

void Player::drawRopeTarget(std::shared_ptr<jt::RenderTargetInterface> targetContainer)
{
    m_targetLine->draw(targetContainer);
}

bool Player::canJump() const { return false; }
