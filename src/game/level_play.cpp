#include "level_play.hpp"

#include <algorithm>
#include <cmath>

namespace game {

LevelPlay::LevelPlay(const LevelData& level, float overviewSeconds)
    : m_world(level),
      m_bonus(0),
      m_overviewTicks(std::max(1, static_cast<int>(std::lround(overviewSeconds * kTicksPerSecond)))),
      m_visibleRect({0, 0}, m_world.size())
{
    m_time.total = level.timeSeconds;
    m_time.left = level.timeSeconds;
}

float LevelPlay::overviewProgress() const
{
    if (m_status != LevelStatus::Overview)
        return 1.0f;
    return static_cast<float>(m_overviewTick) / static_cast<float>(m_overviewTicks);
}

LevelResult LevelPlay::result() const
{
    LevelResult r;
    r.score = m_score;
    if (m_status == LevelStatus::Won)
        r.timeBonus = kTimeBonusPerSecond * m_time.left;
    return r;
}

void LevelPlay::tick()
{
    if (m_status == LevelStatus::Overview) {
        // the game timer only starts after zooming in (Level::overviewZooming())
        if (++m_overviewTick >= m_overviewTicks)
            m_status = LevelStatus::Playing;
        return;
    }
    if (m_status != LevelStatus::Playing)
        return;

    // same order as MyGraphicsScene::timerEvent(): frame counter and bark waves,
    // Level::update() (time, bonus), then the characters
    m_world.beginTick();

    if (m_world.tickCount() % kTicksPerSecond == 0) {
        checkTime();
        if (over())
            return;
    }
    m_bonus.update(m_world, m_time, m_visibleRect);

    const int before = m_world.sheepLeft();
    m_world.advanceCharacters();
    m_score += kSheepScore * (before - m_world.sheepLeft());

    if (m_world.sheepTotal() > 0 && m_world.sheepLeft() == 0)
        finish(LevelStatus::Won);
}

void LevelPlay::checkTime()
{
    if (m_time.bonus == 0)
        --m_time.left;
    else
        --m_time.bonus;

    if (m_time.left <= 0) {
        m_time.left = 0;
        finish(LevelStatus::TimeUp);
    }
}

void LevelPlay::finish(LevelStatus status)
{
    m_status = status;
    m_bonus.reset(m_world);
    m_world.pushEvent(status == LevelStatus::Won ? GameEvent::LevelWon : GameEvent::TimeUp);
}

} // namespace game
