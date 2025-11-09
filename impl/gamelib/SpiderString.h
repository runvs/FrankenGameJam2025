#ifndef FRANKENGAMEJAM2025_SPIDERSTRING_H
#define FRANKENGAMEJAM2025_SPIDERSTRING_H
#include "game_object.hpp"
#include "platform_player.hpp"

#include <memory>

class SpiderString : public jt::GameObject {
public:
    SpiderString(std::shared_ptr<jt::Box2DWorldInterface> const& world, b2Body* from, b2Body* to);

    std::shared_ptr<b2Joint> m_distance_joint;
    std::shared_ptr<b2Joint> m_rope_joint;

    void doUpdate(float elapsed) override;
    void doDraw() const override;

    void withTargetCircle();
    void setStringColor(jt::Color const& c);

private:
    std::shared_ptr<jt::Line> m_line;
    std::shared_ptr<jt::Shape> m_targetCircle;
};

#endif // FRANKENGAMEJAM2025_SPIDERSTRING_H
