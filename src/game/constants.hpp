#pragma once

#include "core/vec2.hpp"

// Gameplay constants, taken over from the Qt version (source/globals.hpp etc.).
// Speeds and distances are per tick; the simulation runs at a fixed kTicksPerSecond.
namespace game {

inline constexpr int kTicksPerSecond = 24;

// --- image sizes in level units (size of the source SVGs in img/) -------------
// The simulation needs them for collision areas; tests check them against the SVGs.
inline constexpr core::Vec2 kDogImageSize{30.0f, 84.000008f};
inline constexpr core::Vec2 kSheepImageSize{41.820915f, 59.999996f};
inline constexpr core::Vec2 kBushImageSize{59.967339f, 59.983982f};
inline constexpr core::Vec2 kTreeImageSize{104.99441f, 104.92934f};
inline constexpr core::Vec2 kExitImageSize{90.000031f, 89.997154f};

inline constexpr int kNumDogAnimationSteps = 7;
inline constexpr int kNumSheepAnimationSteps = 1;

// --- characters ----------------------------------------------------------------
// Highest spatial resolution handled, e.g. when finding the exact distance until a collision.
inline constexpr float kSpeedResolution = 1.0f;
inline constexpr float kZeroSpeedThreshold = 0.01f;
// Images face down; rotation = -angle - offset (as QGraphicsItem::setRotation).
inline constexpr float kCharacterRotationOffset = 90.0f;

// --- dog -----------------------------------------------------------------------
inline constexpr float kNormalDogSpeed = 11.0f;
inline constexpr float kMaxBarkPower = 100.0f;
inline constexpr float kNeededBarkPower = 20.0f;
inline constexpr float kBarkWaveWidth = 25.0f;
inline constexpr float kBarkWaveDuration = 0.3f; // seconds

// --- sheep ---------------------------------------------------------------------
inline constexpr float kNormalSheepSpeed = 1.5f;
// a blocked sheep looks for a way out every second
inline constexpr int kBlockedSheepRetryTicks = kTicksPerSecond;

// --- level elements --------------------------------------------------------------
inline constexpr float kFenceWidth = 13.0f;     // pole diameter
inline constexpr float kFenceSlatWidth = 7.0f;
inline constexpr float kMinFenceSegmentLength = 60.0f;
inline constexpr float kMinWaterSize = 20.0f;

// Values that bonus modes change at runtime (globals in the Qt version).
struct Tuning {
    float dogSpeed = kNormalDogSpeed;
    float normalSheepSpeed = kNormalSheepSpeed;
    float maxSheepSpeed = kNormalDogSpeed;
    float sheepSlowDown = 0.06f;    // per tick, after being barked at
    float barkPowerReload = 0.17f;  // per tick
    float maxBarkDist = 200.0f;
    float barkIntensity = 250.0f;   // sheep speed factor: intensity / distance
};

} // namespace game
