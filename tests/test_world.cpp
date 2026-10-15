// Tests for the simulation (src/game/world, character, element) without any graphics.
#include "core/random.hpp"
#include "game/world.hpp"

#include <cmath>
#include <cstdio>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>

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
bool near(Vec2 a, Vec2 b, float eps = 1e-3f) { return near(a.x, b.x, eps) && near(a.y, b.y, eps); }
bool near(const Rect& a, const Rect& b, float eps = 1e-3f)
{
    return near(a.pos(), b.pos(), eps) && near(a.size(), b.size(), eps);
}

std::string readFile(const std::string& path)
{
    std::ifstream in(path, std::ios::binary);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

LevelData emptyLevel()
{
    LevelData l;
    l.width = 1000;
    l.height = 1000;
    l.timeSeconds = 60;
    return l;
}

bool hasEvent(const std::vector<GameEvent>& events, GameEvent e)
{
    for (GameEvent x : events)
        if (x == e)
            return true;
    return false;
}

// The simulation hard-codes image sizes; they must match the SVGs.
void testImageSizesMatchSvgs()
{
    const struct {
        const char* file;
        Vec2 size;
    } images[] = {{"dog01", kDogImageSize},   {"sheep01", kSheepImageSize}, {"bush1", kBushImageSize},
                  {"tree1", kTreeImageSize}, {"exit1", kExitImageSize},
                  {"bonus/box", kBonusBoxImageSize}};
    const std::regex attr(R"re(<svg\b[^>]*?\swidth="([0-9.]+)"[^>]*?\sheight="([0-9.]+)")re");
    for (const auto& img : images) {
        const std::string svg = readFile(std::string(SHEEPDOG_SOURCE_DIR) + "/img/" + img.file + ".svg");
        std::smatch m;
        CHECK(std::regex_search(svg, m, attr));
        if (m.size() == 3) {
            CHECK(near(std::stof(m[1].str()), img.size.x));
            CHECK(near(std::stof(m[2].str()), img.size.y));
        }
    }
}

void testElements()
{
    // horizontal fence, as level0's first fence: poles at (20,7) and (107,7)
    const Element h = makeFence({20, 7}, {107, 7});
    CHECK(!h.vertical && h.layer == 3);
    CHECK(near(h.bounds, Rect(13.5f, 0.5f, 100, 13)));
    // collision area: slat width, from outer pole edge to outer pole edge; covers the slat
    CHECK(near(*h.collision, Rect(13.5f, 3.5f, 100, 7)));
    CHECK(h.collision->contains(h.slat));
    CHECK(near(h.slat, Rect(20, 3.5f, 87, 7)));
    CHECK(h.poles.size() == 2); // 87 / 60 -> 1 segment
    CHECK(near(h.poles[1], {107, 7}));

    // vertical fence given bottom-up; 130 long -> 2 segments, 3 poles
    const Element v = makeFence({50, 150}, {50, 20});
    CHECK(v.vertical);
    CHECK(near(v.bounds, Rect(43.5f, 13.5f, 13, 143)));
    CHECK(near(*v.collision, Rect(46.5f, 13.5f, 7, 143)));
    CHECK(v.collision->contains(v.slat));
    CHECK(v.poles.size() == 3 && near(v.poles[1], {50, 85}));

    CHECK(!makeWater({0, 0}, {10, 100}, 1).has_value()); // too narrow
    const auto water = makeWater({100, 80}, {10, 10}, 1);
    CHECK(water && near(water->bounds, Rect(10, 10, 90, 70)) && near(*water->collision, water->bounds));

    CHECK(near(*makeBush({10, 20}, 1).collision, Rect(18, 28, 24, 24)));
    CHECK(near(*makeTree({10, 20}, 1).collision, Rect(38, 48, 15, 15)));
    CHECK(near(*makeExit({10, 20}, 1).collision, Rect({10, 20}, kExitImageSize)));
    CHECK(makeBush({0, 0}, 1).layer == 1 && makeExit({0, 0}, 1).layer == 2 && makeTree({0, 0}, 1).layer == 3);
}

void testCharacterGeometry()
{
    const Dog dog({203, 120});
    // collision area: square with half size (30 + 84) / 6 = 19 around the point 26 above the image bottom
    CHECK(near(dog.collisionRadius(), 19.0f));
    CHECK(near(dog.collisionRect().center(), {203, 120 + 42 - 26}));
    CHECK(near(dog.rotationPivot(), {15, 58}));
    CHECK(dog.speed() == 0.0f && dog.barkPower() == kMaxBarkPower);

    const Sheep sheep({100, 100}, 1.5f, 90.0f, 1000);
    CHECK(near(sheep.collisionRect().center(), {100, 100}));
    CHECK(near(sheep.collisionRadius(), (kSheepImageSize.x + kSheepImageSize.y) / 4.0f));
    CHECK(near(sheep.velocity(), {0, -1.5f})); // 90 degrees = up
    CHECK(near(sheep.rotationDeg(), -180.0f));
}

void testSpeedAndAngleSemantics()
{
    Sheep s({0, 0}, 2.0f, 0.0f, 1000);
    CHECK(near(s.velocity(), {2, 0}));
    s.changeAngle(450.0f); // normalised to 90
    CHECK(near(s.angle(), 90.0f) && near(s.velocity(), {0, -2}));
    s.changeAngle(-90.0f);
    CHECK(near(s.angle(), 270.0f) && near(s.velocity(), {0, 2}));

    // zero speed keeps the angle; changeSpeed restarts in that direction
    s.changeSpeed(0.0f);
    CHECK(s.velocity().isNull());
    s.changeAngle(180.0f);
    CHECK(s.velocity().isNull());
    s.changeSpeed(3.0f);
    CHECK(near(s.velocity(), {-3, 0}));

    // dog keyboard quirk: releasing a key clears one component only
    Dog d({0, 0});
    d.setSpeedAndAngle(10.0f, 45.0f);
    d.zeroSpeedX();
    CHECK(near(d.velocity().x, 0.0f) && d.velocity().y < 0.0f && near(d.angle(), 45.0f));
}

void testDogAnimation()
{
    Dog d({0, 0});
    std::string seq;
    for (int i = 0; i < 14; ++i) {
        seq += static_cast<char>('0' + d.animationStep());
        d.updateAnimation();
    }
    CHECK(seq == "01234565432101");
}

void testCollisionStep()
{
    LevelData l = emptyLevel();
    l.dogs = {{100, 100}};
    l.sheep = {{500, 500}};
    const World w(l);
    // the smaller collision area (dog: 2 * 19) minus one
    CHECK(near(w.collisionStep(), 37.0f));
}

void testSheepWalksAndStopsAtFence()
{
    core::seedRandom(1);
    LevelData l = emptyLevel();
    l.sheep = {{500, 500}};
    l.fences = {{{400, 400}, {400, 600}}}; // vertical fence left of the sheep
    World w(l);
    Sheep& s = w.sheep()[0];
    s.setSpeedAndAngle(1.5f, 180.0f); // walk left

    w.tick();
    CHECK(near(s.position(), {498.5f, 500}));

    // keep walking: it must never enter the fence, and turn away when it arrives there
    const Rect fence = *w.elements()[0].collision;
    bool turned = false;
    for (int i = 0; i < 200; ++i) {
        w.tick();
        CHECK(!s.collisionRect().intersects(fence));
        turned |= s.velocity().x >= 0.0f;
    }
    CHECK(turned);
}

void testSheepRescuedAtExit()
{
    LevelData l = emptyLevel();
    l.sheep = {{200, 545}, {800, 800}};
    l.exits = {{{50, 500}, 1}}; // exit right edge at x = 140
    World w(l);
    w.sheep()[0].setSpeedAndAngle(1.5f, 180.0f);
    w.sheep()[1].setSpeedAndAngle(0.0f, 0.0f);
    CHECK(w.sheepLeft() == 2);

    std::vector<GameEvent> events;
    for (int i = 0; i < 60 && !w.sheep()[0].rescued; ++i) {
        w.tick();
        for (GameEvent e : w.takeEvents())
            events.push_back(e);
    }
    CHECK(w.sheep()[0].rescued);
    CHECK(w.sheepLeft() == 1 && w.sheepTotal() == 2);
    CHECK(hasEvent(events, GameEvent::SheepRescued));

    // rescued sheep no longer move or block the dog
    const Vec2 p = w.sheep()[0].position();
    w.tick();
    CHECK(w.sheep()[0].position() == p);
}

void testDogStopsAtFenceAndAtTarget()
{
    LevelData l = emptyLevel();
    l.dogs = {{100, 300}};
    l.sheep = {{900, 900}};
    l.fences = {{{300, 100}, {300, 500}}};
    World w(l);
    Dog& d = w.dogs()[0];

    w.walkTo(0, {600, 300}); // behind the fence
    for (int i = 0; i < 40; ++i)
        w.tick();
    CHECK(d.speed() == 0.0f);
    CHECK(!d.collisionRect().intersects(*w.elements()[0].collision));
    // stopped right in front of it (within the exact-distance resolution)
    CHECK(w.elements()[0].collision->left() - d.collisionRect().right() <= kSpeedResolution + 0.01f);

    // free target: reached within one step, then the target is cleared
    w.walkTo(0, {100, 600});
    for (int i = 0; i < 60 && d.target; ++i)
        w.tick();
    CHECK(!d.target.has_value() && d.speed() == 0.0f);
    CHECK(distance(d.position(), {100, 600}) <= kNormalDogSpeed);
}

void testBark()
{
    LevelData l = emptyLevel();
    l.dogs = {{500, 500}};
    l.sheep = {{600, 500}, {900, 900}}; // near and far (outside bark distance)
    World w(l);
    Sheep& nearSheep = w.sheep()[0];
    Sheep& farSheep = w.sheep()[1];
    const float farAngle = farSheep.angle();

    CHECK(w.bark());
    const auto events = w.takeEvents();
    CHECK(hasEvent(events, GameEvent::Bark));
    CHECK(near(w.dogs()[0].barkPower(), kMaxBarkPower - kNeededBarkPower));

    // speed: intensity / (truncated) distance * old speed, capped; direction away from the dog
    // speed: + intensity / distance (centre to centre) * normal speed, capped; direction away from the dog
    CHECK(near(nearSheep.speed(), std::min(kNormalSheepSpeed + 250.0f / 100.0f * kNormalSheepSpeed, kNormalDogSpeed)));
    CHECK(near(nearSheep.angle(), 0.0f));
    CHECK(nearSheep.velocity().x > 0.0f);
    CHECK(near(farSheep.angle(), farAngle) && near(farSheep.speed(), kNormalSheepSpeed));

    // bark wave from the collision area edge to the bark distance in 0.3 s
    CHECK(w.barkWaves().size() == 1);
    CHECK(near(w.barkWaves()[0].radius, 19.0f));
    int ticks = 0;
    while (!w.barkWaves().empty() && ticks < 100) {
        w.tick();
        ++ticks;
    }
    CHECK(ticks == 8); // 0.3 s * 24 = 7.2 ticks of growth

    // sheep slow down to normal speed again
    for (int i = 0; i < 400; ++i)
        w.tick();
    CHECK(near(nearSheep.speed(), kNormalSheepSpeed) || nearSheep.speed() == 0.0f);
}

void testBarkPower()
{
    LevelData l = emptyLevel();
    l.dogs = {{500, 500}};
    l.sheep = {{900, 900}};
    World w(l);
    for (int i = 0; i < 3; ++i)
        CHECK(w.bark());
    w.takeEvents();
    CHECK(w.bark()); // 100 - 4 * 20 = 20 left: critical
    CHECK(hasEvent(w.takeEvents(), GameEvent::BarkPowerCritical));
    CHECK(w.bark());
    CHECK(!w.bark()); // nothing left
    CHECK(hasEvent(w.takeEvents(), GameEvent::NoBark));

    // reload: 0.17 per tick, event when full
    bool full = false;
    int ticks = 0;
    while (!full && ticks < 1000) {
        w.tick();
        ++ticks;
        full = hasEvent(w.takeEvents(), GameEvent::BarkPowerFull);
    }
    CHECK(full && w.dogs()[0].barkPower() == kMaxBarkPower);
    CHECK(ticks == static_cast<int>(std::ceil(100.0f / 0.17f)));
}

void testSleepingSheepIgnoreBark()
{
    LevelData l = emptyLevel();
    l.dogs = {{500, 500}};
    l.sheep = {{600, 500}, {500, 600}};
    World w(l);
    // only the second sheep sleeps: the first still reacts (the Qt version checked only the first)
    w.sheep()[1].sleeping = true;
    const Vec2 v = w.sheep()[1].velocity();
    CHECK(w.bark());
    CHECK(w.sheep()[1].velocity() == v);
    CHECK(w.sheep()[0].speed() > kNormalSheepSpeed);
    w.tick();
    CHECK(w.sheep()[1].position() == Vec2(500, 600));

    // and the other way round
    World w2(l);
    w2.sheep()[0].sleeping = true;
    CHECK(w2.bark());
    CHECK(near(w2.sheep()[0].speed(), kNormalSheepSpeed));
    CHECK(w2.sheep()[1].speed() > kNormalSheepSpeed);
}

void testStandingSheepReactToBark()
{
    LevelData l = emptyLevel();
    l.dogs = {{500, 500}};
    l.sheep = {{500, 380}, {500, 350}}; // above the dog, 120 and 150 away
    World w(l);
    for (Sheep& s : w.sheep())
        s.setSpeedAndAngle(0.0f, 0.0f);
    CHECK(w.bark());
    // closer means faster, pushed straight up (away from the dog)
    CHECK(near(w.sheep()[0].speed(), 250.0f / 120.0f * kNormalSheepSpeed));
    CHECK(w.sheep()[0].speed() > w.sheep()[1].speed() && w.sheep()[1].speed() > 0.0f);
    CHECK(near(w.sheep()[0].angle(), 90.0f));

    // capped at the maximum sheep speed when very close
    LevelData l2 = emptyLevel();
    l2.dogs = {{500, 500}};
    l2.sheep = {{530, 500}};
    World w2(l2);
    CHECK(w2.bark());
    CHECK(near(w2.sheep()[0].speed(), kNormalDogSpeed));
}

// A sheep walking diagonally into a bush corner touches neither side head-on; it must turn
// away instead of stopping as if blocked on all sides.
void testCornerBounce()
{
    LevelData l = emptyLevel();
    l.bushes = {{{600, 600}, 1}}; // collision area 608..632
    l.sheep = {{570, 570}};       // collision area up to 595.45: diagonal gap of ~12.5
    World w(l);
    Sheep& s = w.sheep()[0];
    s.setSpeedAndAngle(1.5f, 315.0f); // down-right, straight at the corner
    const Rect bush = *w.elements()[0].collision;
    bool bounced = false;
    for (int i = 0; i < 48; ++i) {
        w.tick();
        CHECK(!s.collisionRect().intersects(bush));
        CHECK(!s.blocked);
        bounced |= !near(s.angle(), 315.0f);
    }
    CHECK(bounced);
    CHECK(s.speed() > 0.0f);
}

void testBlockedSheepRetry()
{
    // Sheep in a box only ~1 unit larger than its collision area (50.9): no direction leaves room
    // for a step of 1.5. Fence collision rects span pole +- 3.5, so the inside is [473.5, 525.5]
    // for poles at 470 and 529.
    LevelData l = emptyLevel();
    l.sheep = {{499.5f, 499.5f}};
    l.fences = {{{470, 470}, {529, 470}}, {{470, 529}, {529, 529}}, {{470, 470}, {470, 529}}, {{529, 470}, {529, 529}}};
    World w(l);
    Sheep& s = w.sheep()[0];

    w.tick();
    CHECK(s.blocked && s.speed() == 0.0f);

    // retries once per second: speed is restored on that tick, then it's blocked again
    long retries = 0;
    for (int i = 0; i < 3 * kTicksPerSecond; ++i) {
        const bool before = s.blocked;
        w.tick();
        if (before && w.tickCount() % kBlockedSheepRetryTicks == 0)
            ++retries;
        CHECK(s.blocked);
        CHECK(s.position().x > 473.5f && s.position().x < 525.5f);
    }
    CHECK(retries == 3);
}

// Long random run on a real level: no character walks into an element or another character
// it wasn't already overlapping at the start.
void testLevelRunInvariants()
{
    core::seedRandom(42);
    LevelData l;
    l.width = 503;
    l.height = 420;
    l.timeSeconds = 120;
    // a small pen like in level 0, with the dog inside
    l.fences = {{{100, 100}, {400, 100}}, {{100, 100}, {100, 350}}, {{100, 350}, {400, 350}}, {{400, 100}, {400, 350}}};
    l.bushes = {{{200, 200}, 1}};
    l.trees = {{{300, 150}, 1}};
    l.dogs = {{170, 300}};
    l.sheep = {{250, 160}, {330, 290}, {180, 180}};
    World w(l);

    auto overlaps = [&](const Character& c) {
        for (const Element& e : w.elements())
            if (e.collision && e.collision->intersects(c.collisionRect()))
                return true;
        return false;
    };
    for (const Sheep& s : w.sheep())
        CHECK(!overlaps(s));

    for (int i = 0; i < 3000; ++i) {
        if (i % 50 == 0)
            w.bark();
        if (i % 120 == 0)
            w.walkTo(0, {core::randomFloat(120, 380), core::randomFloat(120, 330)});
        w.tick();
        for (const Sheep& s : w.sheep()) {
            CHECK(!overlaps(s));
            CHECK(!s.collisionRect().intersects(w.dogs()[0].collisionRect()));
        }
        CHECK(!overlaps(w.dogs()[0]));
    }
}

} // namespace

int main()
{
    testImageSizesMatchSvgs();
    testElements();
    testCharacterGeometry();
    testSpeedAndAngleSemantics();
    testDogAnimation();
    testCollisionStep();
    testSheepWalksAndStopsAtFence();
    testSheepRescuedAtExit();
    testDogStopsAtFenceAndAtTarget();
    testBark();
    testBarkPower();
    testSleepingSheepIgnoreBark();
    testStandingSheepReactToBark();
    testCornerBounce();
    testBlockedSheepRetry();
    testLevelRunInvariants();

    if (gFailures == 0)
        std::printf("all world tests passed\n");
    return gFailures == 0 ? 0 : 1;
}
