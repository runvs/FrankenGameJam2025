#include "SpiderString.h"

#include "conversions.hpp"
#include "drawable_helpers.hpp"
#include "game_properties.hpp"

#include <iostream>

SpiderString::SpiderString(
    std::shared_ptr<jt::Box2DWorldInterface> const& world, b2Body* const from, b2Body* const to)
{
    auto dJoint = b2DistanceJointDef {};
    dJoint.bodyA = from;
    dJoint.localAnchorA = b2Vec2 { 0.0, 0.0 };
    dJoint.bodyB = to;
    dJoint.localAnchorB = b2Vec2 { 0.0, 0.0 };

    auto const fullLength = (from->GetPosition() - to->GetPosition()).Length();
    dJoint.length = fullLength * GP::PhysicsInitialRopeLengthFraction(fullLength);
    dJoint.frequencyHz = GP::PhysicsStringFrequency();
    dJoint.dampingRatio = GP::PhysicsStringDampingRatio();

    auto const distanceJoint = world->createJoint(&dJoint);
    m_line = std::make_shared<jt::Line>();
    m_distance_joint = std::shared_ptr<b2Joint>(
        distanceJoint, [world](b2Joint* joint) { world->destroyJoint(joint); });
}

void SpiderString::doUpdate(float elapsed)
{
    // auto a = m_distance_joint->GetBodyA()->GetPosition();
    // auto b = m_distance_joint->GetBodyB()->GetPosition();
    // std::cout << a.x << "|" << a.y << "<-->" << b.x << "|" << b.y << std::endl;
    auto spiderPos = jt::Conversion::vec(m_distance_joint->GetBodyA()->GetPosition());
    auto targetPos = jt::Conversion::vec(m_distance_joint->GetBodyB()->GetPosition());

    m_line->setPosition(spiderPos);
    m_line->setLineVector(targetPos - spiderPos);

    m_line->update(elapsed);
    m_targetCircle->update(elapsed);
}

void SpiderString::withTargetCircle()
{
    m_targetCircle = jt::dh::createShapeCircle(2.0, jt::Color { 255, 0, 0, 255 }, textureManager());
    m_targetCircle->setPosition(jt::Conversion::vec(m_distance_joint->GetBodyB()->GetPosition()));
    m_targetCircle->setOffset(jt::OffsetMode::CENTER);
}

void SpiderString::setStringColor(jt::Color const& c)
{
    m_line->setColor(c);
    m_targetCircle->setColor(c);
}

void SpiderString::doDraw() const
{
    if (m_targetCircle != nullptr) {
        m_targetCircle->draw(renderTarget());

        m_line->draw(renderTarget());
    }
}
