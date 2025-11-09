#ifndef DEMO_STATE_PLATFORMER_HPP
#define DEMO_STATE_PLATFORMER_HPP

#include "player_type.hpp"
#include "screeneffects/scanlines.hpp"
#include <box2dwrapper/box2d_object.hpp>
#include <box2dwrapper/box2d_world_interface.hpp>
#include <game_state.hpp>
#include <level.hpp>
#include <particle_system.hpp>
#include <platform_player.hpp>
#include <screeneffects/vignette.hpp>
#include <shape.hpp>

class SpiderString;

class StatePlatformer : public jt::GameState {
public:
    explicit StatePlatformer(PlayerType pt, std::string const& levelName = "platformer_0_0.json");

private:
    PlayerType m_playerType;
    std::shared_ptr<jt::Box2DWorldInterface> m_world { nullptr };

    std::string m_levelName { "" };
    std::shared_ptr<jt::Shape> m_background { nullptr };
    std::shared_ptr<Level> m_level { nullptr };
    std::shared_ptr<Player> m_player { nullptr };
    std::shared_ptr<jt::Vignette> m_vignette { nullptr };
    std::shared_ptr<jt::ScanLines> m_scanlines { nullptr };

    std::shared_ptr<jt::ParticleSystem<jt::Shape, 50>> m_walkParticles { nullptr };
    std::shared_ptr<jt::ParticleSystem<jt::Shape, 50>> m_playerJumpParticles { nullptr };

    std::array<std::shared_ptr<SpiderString>, 4> m_activeStrings {};
    std::array<std::shared_ptr<jt::Box2DObject>, 4> m_tempStringAnchors {};
    std::array<std::shared_ptr<jt::ParticleSystem<jt::Shape, 100>>, 4> m_stringParticleSystems {};
    std::array<std::shared_ptr<jt::ParticleSystem<jt::Shape, 100>>, 4>
        m_stringLengthParticleSystems {};

    std::shared_ptr<jt::Shape> m_overlay;

    bool m_ending { false };

    std::string getName() const override;
    jt::Color getStringColor(int stringIndex);
    void shootString(int stringIndex, jt::Vector2f direction);

    void onCreate() override;
    void onEnter() override;
    void onUpdate(float const /*elapsed*/) override;
    void onDraw() const override;

    void CreatePlayer();
    void loadLevel();
    void handleCameraScrolling(float const elapsed);
    void endGame();
    void createPlayerWalkParticles();
    void createPlayerJumpParticleSystem();
    std::shared_ptr<jt::ParticleSystem<jt::Shape, 100>> createStringParticles(int index);
    std::shared_ptr<jt::ParticleSystem<jt::Shape, 100>> createStringLengthParticles(int index);
};

#endif // DEMO_STATE_PLATFORMER_HPP
