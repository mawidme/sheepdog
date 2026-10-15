#pragma once

#include "core/rect.hpp"
#include "core/vec2.hpp"

// Bonus boxes and modes (Qt: Bonus). Every 6-11 s a box appears somewhere in the visible part
// of the level; the dog picks it up by walking over it, which activates its mode for a while.
namespace game {

class World;

enum class BonusMode { Drunk, Sleep, SheepOnSpeed, TimeBonus, PowerBark, Count };

// level time, shared with LevelPlay (the time bonus mode refills it)
struct LevelTime {
    int total = 0;  // seconds
    int left = 0;
    int bonus = 0;  // extra seconds, used up before `left`
};

class Bonus {
public:
    explicit Bonus(long nowTick);

    // once per tick while playing, before the characters move
    void update(World& world, LevelTime& time, const core::Rect& visibleLevelRect);
    // ends the active mode, removes the box (level finished or restarted)
    void reset(World& world);

    // --- state for drawing / input ---------------------------------------------
    bool boxVisible() const { return m_boxVisible; }
    const core::Rect& boxRect() const { return m_boxRect; }
    BonusMode mode() const { return m_mode; } // mode of the current box or active bonus
    bool surprise() const { return m_surprise; } // box doesn't show its mode

    // the picked-up mode while it lasts
    bool active() const { return m_pickedUp; }
    bool active(BonusMode m) const { return m_pickedUp && m_mode == m; }

    // mode image shown at the pickup position for a while: grows and fades out
    bool modeImageVisible() const { return m_modeImageVisible; }
    core::Vec2 modeImageCenter() const { return m_modeImageCenter; }
    float modeImageScale() const;   // 0.1 .. 1.0
    float modeImageOpacity() const; // 1.0 .. 0.0

    // tests: next box gets this mode (and isn't a surprise)
    void forceNextMode(BonusMode m);

private:
    void scheduleNext(long nowTick);
    void pickMode();
    bool placeBox(const World& world, const core::Rect& visibleLevelRect);
    void activate(World& world, LevelTime& time);
    void applyEachTick(World& world);
    void deactivate(World& world);

    BonusMode m_mode = BonusMode::Drunk;
    bool m_surprise = false;
    bool m_forced = false;

    long m_boxShowTick = 0;
    long m_boxHideTick = 0;
    long m_modeEndTick = 0;
    long m_modeImageHideTick = 0;

    bool m_boxVisible = false;
    bool m_pickedUp = false;
    core::Rect m_boxRect;

    bool m_modeImageVisible = false;
    core::Vec2 m_modeImageCenter;
    float m_modeImageProgress = 0.0f; // 0 .. 1 while the mode image is shown
    float m_modeImageProgressStep = 0.0f;

    // power bark: values to restore
    float m_oldBarkReload = 0.0f;
    float m_oldMaxBarkDist = 0.0f;
    float m_oldBarkIntensity = 0.0f;
    float m_oldMaxSheepSpeed = 0.0f;
};

} // namespace game
