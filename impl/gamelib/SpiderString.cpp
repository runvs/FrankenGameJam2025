#include "SpiderString.h"

#include "conversions.hpp"
#include "drawable_helpers.hpp"

#include <iostream>

SpiderString::SpiderString(
    std::shared_ptr<jt::Box2DWorldInterface> const& world, b2Body* const from, b2Body* const to)
{
    auto dJoint = b2DistanceJointDef {};
    dJoint.bodyA = from;
    dJoint.localAnchorA = b2Vec2 { 0.0, 0.0 };
    dJoint.bodyB = to;
    dJoint.localAnchorB = b2Vec2 { 0.0, 0.0 }; // TODO needs to be local raycast hit point

    dJoint.length = (from->GetPosition() - to->GetPosition()).Length() * 0.75f;
    dJoint.frequencyHz = 0.5;
    dJoint.dampingRatio = 0.01;

    auto const distanceJoint = world->createJoint(&dJoint);
    m_line = std::make_shared<jt::Line>();
    m_distance_joint = std::shared_ptr<b2Joint>(
        distanceJoint, [world](b2Joint* joint) { world->destroyJoint(joint); });

    // auto rJoint = b2RopeJointDef {};
    // rJoint.bodyA = from;
    // rJoint.localAnchorA = dJoint.localAnchorA;
    // rJoint.bodyB = to;
    // rJoint.localAnchorB = dJoint.localAnchorB;
    // rJoint.maxLength = 50.0;
    // auto const ropeJoint = world->createJoint(&rJoint);
    // m_rope_joint = std::shared_ptr<b2Joint>(ropeJoint, [world](b2Joint* joint) {
    // world->destroyJoint(joint);
    // });
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
    if (m_debugCircle != nullptr) {
        m_debugCircle->update(elapsed);
    }
}

void SpiderString::withDebugCircle()
{
    m_debugCircle = jt::dh::createShapeCircle(2.0, jt::Color { 255, 0, 0, 255 }, textureManager());

    m_debugCircle->setPosition(jt::Conversion::vec(m_distance_joint->GetBodyB()->GetPosition()));
    m_debugCircle->setOffset(jt::OffsetMode::CENTER);
}

void SpiderString::doDraw() const
{
    if (m_debugCircle != nullptr) {
        m_debugCircle->draw(renderTarget());

        m_line->draw(renderTarget());
    }
}
