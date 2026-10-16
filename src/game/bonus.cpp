#include "bonus.hpp"

#include "constants.hpp"
#include "core/random.hpp"
#include "world.hpp"

#include <algorithm>

using core::Rect;
using core::Vec2;

namespace game {

namespace {

constexpr int kModeDurationSeconds[static_cast<int>(BonusMode::Count)] = {10, 6, 10, 10, 15};
constexpr int kBoxMinDelaySeconds = 6;
constexpr int kBoxMaxDelaySeconds = 11;
constexpr float kBoxShowSeconds = 3.5f;
constexpr int kModeImageSeconds = 2;
constexpr float kMinModeImageScale = 0.1f;
constexpr float kMaxModeImageScale = 1.0f;
// probability that a box hides its mode: 1 : kSurpriseRatio
constexpr int kSurpriseRatio = 3;
constexpr int kTimeBonusSeconds = 15;
constexpr float kSheepOnSpeedFactor = 2.0f;

constexpr float kPowerBarkReload = 1.0f;
constexpr float kPowerBarkDist = 400.0f;
constexpr float kPowerBarkIntensity = 500.0f;
constexpr float kPowerBarkSlowDown = 0.12f;

// attempts to find a free spot for a box before giving up on this one
constexpr int kMaxPlacementAttempts = 1000;

} // namespace

Bonus::Bonus(long nowTick)
{
    scheduleNext(nowTick);
}

void Bonus::forceNextMode(BonusMode m)
{
    m_mode = m;
    m_surprise = false;
    m_forced = true;
}

void Bonus::pickMode()
{
    if (m_forced) {
        m_forced = false;
        return;
    }
    m_mode = static_cast<BonusMode>(core::randomInt(0, static_cast<int>(BonusMode::Count) - 1));
    m_surprise = core::randomInt(1, kSurpriseRatio) == 1;
}

void Bonus::scheduleNext(long nowTick)
{
    pickMode();
    m_boxShowTick = nowTick + core::randomInt(kBoxMinDelaySeconds * kTicksPerSecond, kBoxMaxDelaySeconds * kTicksPerSecond);
    m_boxHideTick = m_boxShowTick + static_cast<long>(kBoxShowSeconds * kTicksPerSecond);
}

float Bonus::modeImageScale() const
{
    return kMinModeImageScale + (kMaxModeImageScale - kMinModeImageScale) * m_modeImageProgress;
}

float Bonus::modeImageOpacity() const
{
    return 1.0f - m_modeImageProgress;
}

bool Bonus::placeBox(const World& world, const Rect& visibleLevelRect)
{
    // inside the fenced level area and the visible part of the screen
    const Rect levelRect{kFenceWidth, kFenceWidth, world.size().x - 2 * kFenceWidth - kBonusBoxImageSize.x,
                         world.size().y - 2 * kFenceWidth - kBonusBoxImageSize.y};
    const Rect spot = visibleLevelRect.intersected(levelRect);
    if (spot.isEmpty())
        return false;

    for (int attempt = 0; attempt < kMaxPlacementAttempts; ++attempt) {
        const Rect box{{core::randomFloat(spot.left(), spot.right()), core::randomFloat(spot.top(), spot.bottom())},
                       kBonusBoxImageSize};
        bool collision = false;
        for (const Element& e : world.elements())
            collision |= e.collision && e.collision->intersects(box);
        for (const Dog& d : world.dogs())
            collision |= d.collisionRect().intersects(box);
        if (!collision) {
            m_boxRect = box;
            return true;
        }
    }
    return false;
}

void Bonus::update(World& world, LevelTime& time, const Rect& visibleLevelRect)
{
    const long now = world.tickCount();

    if (!m_boxVisible && now < m_boxShowTick)
        return; // nothing on stage yet

    if (now == m_boxShowTick) {
        // (The Qt version tried forever; give up on this box if there is no free spot.)
        if (placeBox(world, visibleLevelRect)) {
            m_boxVisible = true;
            m_pickedUp = false;
        } else {
            scheduleNext(now);
        }
        return;
    }

    if (m_pickedUp) {
        if (now == m_modeEndTick) {
            deactivate(world);
            scheduleNext(now);
            return;
        }
        applyEachTick(world);
    } else if (now == m_boxHideTick) {
        // not picked up in time. (The Qt version also "reset" the mode that was never active,
        // restoring uninitialised values in the power bark case; nothing to undo here.)
        m_boxVisible = false;
        scheduleNext(now);
    } else if (m_boxVisible && !world.dogs().empty() && world.dogs()[0].collisionRect().intersects(m_boxRect)) {
        m_boxVisible = false;
        activate(world, time);
    }
}

void Bonus::activate(World& world, LevelTime& time)
{
    const long now = world.tickCount();
    m_pickedUp = true;
    world.pushEvent(GameEvent::BonusPickedUp);

    Tuning& t = world.tuning;
    switch (m_mode) {
    case BonusMode::Drunk:
        // controls are inverted by the input layer while active(BonusMode::Drunk)
        break;
    case BonusMode::Sleep:
        // applied every tick (applyEachTick)
        break;
    case BonusMode::SheepOnSpeed:
        t.normalSheepSpeed *= kSheepOnSpeedFactor;
        for (Sheep& s : world.sheep())
            s.changeSpeed(t.normalSheepSpeed);
        break;
    case BonusMode::TimeBonus:
        // refill at most up to the start time
        time.bonus = std::min(kTimeBonusSeconds, time.total - time.left);
        break;
    case BonusMode::PowerBark:
        m_oldBarkReload = t.barkPowerReload;
        m_oldMaxBarkDist = t.maxBarkDist;
        m_oldBarkIntensity = t.barkIntensity;
        m_oldMaxSheepSpeed = t.maxSheepSpeed;
        m_oldSheepSlowDown = t.sheepSlowDown;
        t.barkPowerReload = kPowerBarkReload;
        t.maxBarkDist = kPowerBarkDist;
        t.barkIntensity = kPowerBarkIntensity;
        t.maxSheepSpeed = kNormalDogSpeed;
        t.sheepSlowDown = kPowerBarkSlowDown;
        break;
    case BonusMode::Count:
        break;
    }
    m_modeEndTick = now + kModeDurationSeconds[static_cast<int>(m_mode)] * kTicksPerSecond;

    m_modeImageVisible = true;
    m_modeImageHideTick = now + kModeImageSeconds * kTicksPerSecond;
    m_modeImageProgress = 0.0f;
    m_modeImageProgressStep = 1.0f / static_cast<float>(m_modeImageHideTick - now);
    m_modeImageCenter = world.dogs().empty() ? Vec2() : world.dogs()[0].position();
}

void Bonus::applyEachTick(World& world)
{
    if (m_modeImageVisible) {
        if (world.tickCount() < m_modeImageHideTick)
            m_modeImageProgress += m_modeImageProgressStep;
        else
            m_modeImageVisible = false;
    }

    if (m_mode == BonusMode::Sleep) {
        // stop all sheep
        world.tuning.normalSheepSpeed = 0.0f;
        for (Sheep& s : world.sheep()) {
            if (s.rescued)
                continue;
            s.changeSpeed(0.0f);
            s.sleeping = true;
        }
    }
}

void Bonus::deactivate(World& world)
{
    m_pickedUp = false;
    m_modeImageVisible = false;
    world.pushEvent(GameEvent::BonusEnded);

    Tuning& t = world.tuning;
    switch (m_mode) {
    case BonusMode::Sleep:
        t.normalSheepSpeed = kNormalSheepSpeed;
        for (Sheep& s : world.sheep()) {
            if (s.rescued)
                continue;
            s.sleeping = false;
            s.changeSpeed(t.normalSheepSpeed);
        }
        break;
    case BonusMode::SheepOnSpeed:
        t.normalSheepSpeed = kNormalSheepSpeed;
        for (Sheep& s : world.sheep())
            s.changeSpeed(t.normalSheepSpeed);
        break;
    case BonusMode::PowerBark:
        t.barkPowerReload = m_oldBarkReload;
        t.maxBarkDist = m_oldMaxBarkDist;
        t.barkIntensity = m_oldBarkIntensity;
        t.maxSheepSpeed = m_oldMaxSheepSpeed;
        // (the Qt version kept the faster slow-down for the rest of the level)
        t.sheepSlowDown = m_oldSheepSlowDown;
        break;
    case BonusMode::Drunk:
    case BonusMode::TimeBonus:
    case BonusMode::Count:
        break;
    }
}

void Bonus::reset(World& world)
{
    m_boxVisible = false;
    if (m_pickedUp)
        deactivate(world);
    m_modeImageVisible = false;
}

} // namespace game
