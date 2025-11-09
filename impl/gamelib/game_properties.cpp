#include "game_properties.hpp"
#include <color/palette_builder.hpp>

#include <iostream>

namespace {

jt::Palette createPalette()
{
    jt::PaletteBuilder builder;
    return builder
        .addColorsFromGPL(R"(GIMP Palette
#Palette Name: Dreamscape8
#Description: A palette made of low saturation colours to give your art a dreamlike quality.
#Colors: 8
94	113	142	5e718e
128	128	128	808080
42	42	42	2b2b2b
84	51	68	543344
40	43	90	282b5a
81	82	98	515262
99	120	125	63787d
142	160	145	8ea091
)")
        .create();
}

} // namespace

jt::Palette GP::getPalette()
{
    static auto const p = createPalette();

    return p;
}

int GP::PhysicVelocityIterations() { return 20; }

int GP::PhysicPositionIterations() { return 15; }

jt::Vector2f GP::PlayerSize() { return jt::Vector2f { 16.0f, 16.0f }; }

float GP::PhysicsGravityStrength() { return 200.0f; }

float GP::PhysicsStringFrequency() { return 0.5f; }

float GP::PhysicsStringDampingRatio() { return 0.01f; }

float GP::PhysicsInitialRopeLengthFraction(float ropeLengthInPixel)
{

    auto const v = 0.6f + 0.35f * std::clamp(ropeLengthInPixel / 350.0f, 0.0f, 1.0f);
    std::cout << ropeLengthInPixel << " " << v << "\n";
    return v;
}

float GP::PhysicsStringMaxLengthInPx() { return 350.0f; }
