#ifndef GAME_GAMEPROPERTIES_HPP
#define GAME_GAMEPROPERTIES_HPP

#include <color/color.hpp>
#include <color/palette.hpp>
#include <vector.hpp>
#include <Box2D/Common/b2Settings.h>
#include <string>

class GP {
public:
    GP() = delete;

    static std::string GameName() { return "ArachnoSwing"; }

    static std::string AuthorName() { return "BloodyOrange, Jacudibu Laguna_999, man00ka"; }

    static std::string JamName() { return ""; }

    static std::string JamDate() { return "FrankenGameJam 2025"; }

    static std::string ExplanationText() { return "[A] to start\n[Y] to switch chars"; }

    static jt::Vector2f GetWindowSize() { return jt::Vector2f { 1024, 768 }; }

    static float GetZoom() { return 2.0f; }

    static jt::Vector2f GetScreenSize() { return GetWindowSize() * (1.0f / GetZoom()); }

    static jt::Color PaletteBackground() { return GP::getPalette().getColor(4); }

    static jt::Color PaletteFontFront() { return jt::Color { 210, 205, 209 }; }

    static jt::Color PalleteFrontHighlight() { return GP::getPalette().getColor(1); }

    static jt::Color PaletteFontShadow() { return GP::getPalette().getColor(2); }

    static jt::Color PaletteFontCredits() { return jt::Color { 255, 166, 207 }; }

    static jt::Palette getPalette();

    static int PhysicVelocityIterations();
    static int PhysicPositionIterations();
    static jt::Vector2f PlayerSize();
    static float PhysicsGravityStrength();
    static float PhysicsStringFrequency();
    static float PhysicsStringDampingRatio();
    static float PhysicsInitialRopeLengthFraction(float ropeLengthInPixel);
    static float PhysicsStringMaxLengthInPx();
    static float PhysicsStringBelowShrinkingValue();
};

#endif
