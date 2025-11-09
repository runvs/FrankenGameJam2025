#ifndef JAMTEMPLATE_DEMO_PLATFORM_PLAYER
#define JAMTEMPLATE_DEMO_PLATFORM_PLAYER

#include "line.hpp"
#include "player_type.hpp"
#include <animation.hpp>
#include <box2dwrapper/box2d_object.hpp>
#include <game_object.hpp>
#include <particle_system.hpp>
#include <shape.hpp>
#include <Box2D/Box2D.h>
#include <memory>

class Player : public jt::GameObject {
public:
    using Sptr = std::shared_ptr<Player>;
    Player(PlayerType pt, std::shared_ptr<jt::Box2DWorldInterface> world);

    ~Player() = default;

    std::shared_ptr<jt::Animation> getAnimation();
    b2Body* getB2Body();

    void setTouchesGround(bool touchingGround);

    jt::Vector2f getPosOnScreen() const;
    void setPosition(jt::Vector2f const& pos);
    jt::Vector2f getPosition() const;

    void setWalkParticleSystem(std::weak_ptr<jt::ParticleSystem<jt::Shape, 50>> ps);
    void setJumpParticleSystem(std::weak_ptr<jt::ParticleSystem<jt::Shape, 50>> ps);

    void setLevelSize(jt::Vector2f const& levelSizeInTiles);
    void setStringFireCallback(std::function<void(int, jt::Vector2f const&)> const& callback);

    void drawRopeTarget(std::shared_ptr<jt::RenderTargetInterface> targetContainer);

private:
    PlayerType m_playerType;
    std::shared_ptr<jt::Animation> m_animation;
    std::shared_ptr<jt::Box2DObject> m_physicsObject;
    std::shared_ptr<jt::Line> m_crosshairV;
    std::shared_ptr<jt::Line> m_crosshairH;

    std::shared_ptr<jt::Line> m_targetLine;

    jt::Vector2f m_levelSizeInTiles { 0.0f, 0.0f };

    bool canJump() const;
    void doCreate() override;
    void doUpdate(float elapsed) override;
    void doDraw() const override;

    void handleMovement(float elapsed);
    void updateAnimation(float elapsed);
    void clampPositionToLevelSize(jt::Vector2f& currentPosition) const;
    jt::Vector2f m_crosshairPos { 0.0f, 0.0f };
    jt::Vector2f m_gpAxis { 0.0f, 0.0f };

    std::function<void(int, jt::Vector2f const&)> m_fireStringCallback { nullptr };
};

#endif // JAMTEMPLATE_DEMO_PLATFORM_PLAYER
