#include "SpiderString.h"

#include <iostream>

SpiderString::SpiderString(
    std::shared_ptr<jt::Box2DWorldInterface> const& world,
    b2Body* const from,
    b2Body* const to)
{
    auto jointDefinition = b2DistanceJointDef {};
    jointDefinition.bodyA = from;
    jointDefinition.localAnchorA = b2Vec2 {0.0, 0.0};
    jointDefinition.bodyB = to;
    jointDefinition.localAnchorB = b2Vec2 {0.0, 0.0}; // TODO needs to be local raycast hit point

    jointDefinition.length = 50.0;
    jointDefinition.frequencyHz = 0.5;
    jointDefinition.dampingRatio = 0.1;

    auto const distanceJoint = world->createJoint(&jointDefinition);
    m_joint = std::shared_ptr<b2Joint>(distanceJoint, [world](b2Joint* joint) {
        world->destroyJoint(joint);
    });
}

void SpiderString::update()
{
    // auto a = m_joint->GetBodyA()->GetPosition();
    // auto b = m_joint->GetBodyB()->GetPosition();
    // std::cout << a.x << "|" << a.y << "<-->" << b.x << "|" << b.y << std::endl;
}