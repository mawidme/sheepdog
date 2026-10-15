// Tests for the level rules (src/game/level_play, bonus): overview, time, score, bonuses.
#include "core/random.hpp"
#include "game/level_play.hpp"

#include <cmath>
#include <cstdio>

using core::Rect;
using core::Vec2;
using namespace game;

namespace {

int gFailures = 0;

#define CHECK(cond)                                                              \
    do {                                                                         \
        if (!(cond)) {                                                           \
            std::printf("%s:%d: CHECK failed: %s\n", __FILE__, __LINE__, #cond); \
            ++gFailures;                                                         \
        }                                                                        \
    } while (0)

bool near(float a, float b, float eps = 1e-3f) { return std::fabs(a - b) <= eps; }

bool hasEvent(const std::vector<GameEvent>& events, GameEvent e)
{
    for (GameEvent x : events)
        if (x == e)
            return true;
    return false;
}

// dog in the middle, one sheep far away standing still
LevelData simpleLevel(int seconds = 60)
{
    LevelData l;
    l.width = 800;
    l.height = 600;
    l.timeSeconds = seconds;
    l.dogs = {{400, 300}};
    l.sheep = {{700, 500}};
    return l;
}

void stopSheep(LevelPlay& p)
{
    for (Sheep& s : p.world().sheep())
        s.setSpeedAndAngle(0.0f, 0.0f);
    p.world().tuning.normalSheepSpeed = 0.0f;
}

void skipOverview(LevelPlay& p)
{
    while (p.status() == LevelStatus::Overview)
        p.tick();
}

void testOverview()
{
    LevelPlay p(simpleLevel(), 2.0f);
    CHECK(p.status() == LevelStatus::Overview);
    const Vec2 sheepStart = p.world().sheep()[0].position();
    for (int i = 0; i < 2 * kTicksPerSecond - 1; ++i)
        p.tick();
    CHECK(p.status() == LevelStatus::Overview);
    CHECK(near(p.overviewProgress(), 47.0f / 48.0f));
    CHECK(p.world().tickCount() == 0 && p.world().sheep()[0].position() == sheepStart);
    p.tick();
    CHECK(p.status() == LevelStatus::Playing && p.overviewProgress() == 1.0f);
}

void testTimeUp()
{
    LevelPlay p(simpleLevel(3), 0.0f);
    skipOverview(p);
    stopSheep(p);
    for (int i = 0; i < kTicksPerSecond; ++i)
        p.tick();
    CHECK(p.time().left == 2);
    for (int i = 0; i < 2 * kTicksPerSecond; ++i)
        p.tick();
    CHECK(p.status() == LevelStatus::TimeUp && p.over());
    CHECK(p.time().left == 0);
    CHECK(hasEvent(p.world().takeEvents(), GameEvent::TimeUp));
    CHECK(p.result().timeBonus == 0);

    // nothing happens afterwards
    const long t = p.world().tickCount();
    p.tick();
    CHECK(p.world().tickCount() == t);
}

void testWinAndScore()
{
    LevelData l = simpleLevel(60);
    l.sheep = {{200, 430}, {200, 470}};
    l.exits = {{{50, 400}, 1}}; // x 50..140, y 400..490
    LevelPlay p(l, 0.0f);
    skipOverview(p);
    for (Sheep& s : p.world().sheep())
        s.setSpeedAndAngle(1.5f, 180.0f);

    for (int i = 0; i < 10 * kTicksPerSecond && !p.over(); ++i)
        p.tick();
    CHECK(p.status() == LevelStatus::Won);
    CHECK(p.score() == 2 * kSheepScore);
    const LevelResult r = p.result();
    CHECK(r.score == 20 && r.timeBonus == p.time().left && r.total() == 20 + p.time().left);
    CHECK(p.time().left > 50);
    CHECK(hasEvent(p.world().takeEvents(), GameEvent::LevelWon));
}

// runs until a bonus box appears (6-11 s), puts the dog onto it and returns once picked up
void pickUpBonus(LevelPlay& p, BonusMode mode)
{
    p.bonus().forceNextMode(mode);
    for (int i = 0; i < 12 * kTicksPerSecond && !p.bonus().boxVisible(); ++i)
        p.tick();
    CHECK(p.bonus().boxVisible());
    CHECK(p.bonus().mode() == mode && !p.bonus().surprise());
    Dog& dog = p.world().dogs()[0];
    dog.move(p.bonus().boxRect().center() - dog.collisionRect().center());
    p.tick();
    CHECK(p.bonus().active(mode));
    CHECK(!p.bonus().boxVisible());
    CHECK(hasEvent(p.world().takeEvents(), GameEvent::BonusPickedUp));
}

void waitForBonusEnd(LevelPlay& p)
{
    for (int i = 0; i < 20 * kTicksPerSecond && p.bonus().active(); ++i)
        p.tick();
    CHECK(!p.bonus().active());
}

void testBonusBoxPlacementAndExpiry()
{
    core::seedRandom(5);
    LevelData l = simpleLevel(999); // 20 rounds of up to 14.5 s
    l.fences = {{{100, 100}, {700, 100}}};
    l.bushes = {{{300, 300}, 1}};
    LevelPlay p(l, 0.0f);
    skipOverview(p);
    stopSheep(p);
    const Rect visible{100, 50, 500, 400};
    p.setVisibleRect(visible);
    const Tuning tuningBefore = p.world().tuning;

    for (int round = 0; round < 20; ++round) {
        long ticks = 0;
        while (!p.bonus().boxVisible() && ticks < 12 * kTicksPerSecond) {
            p.tick();
            ++ticks;
        }
        CHECK(p.bonus().boxVisible());
        CHECK(ticks >= 6 * kTicksPerSecond - 1 && ticks <= 11 * kTicksPerSecond + 1);
        // inside the visible part of the level, not on anything
        const Rect box = p.bonus().boxRect();
        CHECK(visible.contains(box.topLeft()));
        for (const Element& e : p.world().elements())
            CHECK(!e.collision->intersects(box));
        CHECK(!p.world().dogs()[0].collisionRect().intersects(box));

        // disappears after 3.5 s if not picked up, without touching the tuning
        ticks = 0;
        while (p.bonus().boxVisible() && ticks < 10 * kTicksPerSecond) {
            p.tick();
            ++ticks;
        }
        CHECK(!p.bonus().boxVisible() && !p.bonus().active());
        CHECK(ticks == 84);
        CHECK(p.world().tuning.barkPowerReload == tuningBefore.barkPowerReload);
    }
}

void testSleepMode()
{
    core::seedRandom(1);
    LevelPlay p(simpleLevel(120), 0.0f);
    skipOverview(p);
    pickUpBonus(p, BonusMode::Sleep);
    p.tick();
    const Sheep& s = p.world().sheep()[0];
    CHECK(s.sleeping && s.speed() == 0.0f);
    const Vec2 pos = s.position();
    CHECK(p.world().bark());
    p.tick();
    CHECK(s.position() == pos);

    waitForBonusEnd(p);
    CHECK(!s.sleeping && near(s.speed(), kNormalSheepSpeed));
    CHECK(near(p.world().tuning.normalSheepSpeed, kNormalSheepSpeed));
}

void testSheepOnSpeedMode()
{
    core::seedRandom(2);
    LevelPlay p(simpleLevel(120), 0.0f);
    skipOverview(p);
    pickUpBonus(p, BonusMode::SheepOnSpeed);
    CHECK(near(p.world().tuning.normalSheepSpeed, 2 * kNormalSheepSpeed));
    waitForBonusEnd(p);
    CHECK(near(p.world().tuning.normalSheepSpeed, kNormalSheepSpeed));
}

void testTimeBonusMode()
{
    core::seedRandom(3);
    LevelPlay p(simpleLevel(120), 0.0f);
    skipOverview(p);
    stopSheep(p);
    pickUpBonus(p, BonusMode::TimeBonus);
    // refilled by up to 15 s, but never above the start time
    const int used = p.time().total - p.time().left;
    CHECK(p.time().bonus == std::min(15, used));
    // bonus seconds are used up first
    const int left = p.time().left;
    for (int i = 0; i < 2 * kTicksPerSecond; ++i)
        p.tick();
    CHECK(p.time().left == left);
}

void testPowerBarkMode()
{
    core::seedRandom(4);
    LevelPlay p(simpleLevel(120), 0.0f);
    skipOverview(p);
    stopSheep(p);
    const Tuning before = p.world().tuning;
    pickUpBonus(p, BonusMode::PowerBark);
    const Tuning& t = p.world().tuning;
    CHECK(t.maxBarkDist == 400.0f && t.barkIntensity == 500.0f && t.barkPowerReload == 1.0f);
    CHECK(t.sheepSlowDown == 0.12f);
    waitForBonusEnd(p);
    CHECK(t.maxBarkDist == before.maxBarkDist && t.barkIntensity == before.barkIntensity);
    CHECK(t.barkPowerReload == before.barkPowerReload && t.maxSheepSpeed == before.maxSheepSpeed);
    CHECK(t.sheepSlowDown == before.sheepSlowDown);
}

void testDrunkModeAndImage()
{
    core::seedRandom(6);
    LevelPlay p(simpleLevel(120), 0.0f);
    skipOverview(p);
    stopSheep(p);
    pickUpBonus(p, BonusMode::Drunk);
    // mode image grows and fades over 2 s at the pickup position
    CHECK(p.bonus().modeImageVisible() && near(p.bonus().modeImageScale(), 0.1f));
    for (int i = 0; i < kTicksPerSecond; ++i)
        p.tick();
    CHECK(p.bonus().modeImageScale() > 0.5f && p.bonus().modeImageOpacity() < 0.6f);
    for (int i = 0; i < 2 * kTicksPerSecond; ++i)
        p.tick();
    CHECK(!p.bonus().modeImageVisible() && p.bonus().active(BonusMode::Drunk));
    waitForBonusEnd(p);
}

void testBonusEndsWithLevel()
{
    core::seedRandom(7);
    LevelPlay p(simpleLevel(15), 0.0f);
    skipOverview(p);
    stopSheep(p);
    pickUpBonus(p, BonusMode::SheepOnSpeed);
    for (int i = 0; i < 20 * kTicksPerSecond && !p.over(); ++i)
        p.tick();
    CHECK(p.status() == LevelStatus::TimeUp);
    CHECK(!p.bonus().active() && near(p.world().tuning.normalSheepSpeed, kNormalSheepSpeed));
}

} // namespace

int main()
{
    testOverview();
    testTimeUp();
    testWinAndScore();
    testBonusBoxPlacementAndExpiry();
    testSleepMode();
    testSheepOnSpeedMode();
    testTimeBonusMode();
    testPowerBarkMode();
    testDrunkModeAndImage();
    testBonusEndsWithLevel();

    if (gFailures == 0)
        std::printf("all level play tests passed\n");
    return gFailures == 0 ? 0 : 1;
}
