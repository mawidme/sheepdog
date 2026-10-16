#pragma once

#include "bonus.hpp"
#include "core/rect.hpp"
#include "level_data.hpp"
#include "world.hpp"

// One attempt at a level: the World plus the level rules of the Qt version's Level class:
// start overview, time limit, score, bonuses, winning and losing. Menus, dialogs and the
// camera are up to the presentation layer.
namespace game {

enum class LevelStatus {
    Overview, // zooming in on the level at the start, simulation not running yet
    Playing,
    Won,      // all sheep rescued
    TimeUp,
};

struct LevelResult {
    int score = 0;     // per rescued sheep
    int timeBonus = 0; // seconds left
    int total() const { return score + timeBonus; }
};

inline constexpr int kSheepScore = 10;
inline constexpr int kTimeBonusPerSecond = 1;
// zoom at the start of the overview, relative to the normal zoom
inline constexpr float kOverviewStartZoom = 0.3f;
inline constexpr float kOverviewSecondsDesktop = 2.0f;
inline constexpr float kOverviewSecondsMobile = 0.5f;

class LevelPlay {
public:
    LevelPlay(const LevelData& level, float overviewSeconds = kOverviewSecondsDesktop);

    // one tick (1 / kTicksPerSecond); does nothing once the level is over
    void tick();

    // Part of the level visible on screen, in level coordinates: bonus boxes appear there.
    void setVisibleRect(const core::Rect& r) { m_visibleRect = r; }

    LevelStatus status() const { return m_status; }
    bool over() const { return m_status == LevelStatus::Won || m_status == LevelStatus::TimeUp; }

    // 0 .. 1 during LevelStatus::Overview, 1 afterwards
    float overviewProgress() const;

    const LevelTime& time() const { return m_time; }
    int score() const { return m_score; }
    // final result once won
    LevelResult result() const;

    World& world() { return m_world; }
    const World& world() const { return m_world; }
    Bonus& bonus() { return m_bonus; }
    const Bonus& bonus() const { return m_bonus; }

private:
    void checkTime();
    void finish(LevelStatus status);

    World m_world;
    Bonus m_bonus;
    LevelTime m_time;
    LevelStatus m_status = LevelStatus::Overview;
    int m_overviewTicks;
    int m_overviewTick = 0;
    int m_score = 0;
    core::Rect m_visibleRect;
};

} // namespace game
