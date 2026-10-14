// Unit tests for src/core. Deliberately framework-free: run the executable,
// non-zero exit code means failure. Registered with CTest.
#include "core/color.hpp"
#include "core/random.hpp"
#include "core/rect.hpp"
#include "core/strings.hpp"
#include "core/vec2.hpp"

#include <cmath>
#include <cstdio>

using namespace core;

namespace {

int gFailures = 0;

#define CHECK(cond)                                                              \
    do {                                                                         \
        if (!(cond)) {                                                           \
            std::printf("%s:%d: CHECK failed: %s\n", __FILE__, __LINE__, #cond); \
            ++gFailures;                                                         \
        }                                                                        \
    } while (0)

bool near(float a, float b, float eps = 1e-4f) { return std::fabs(a - b) <= eps; }
bool near(Vec2 a, Vec2 b, float eps = 1e-4f) { return near(a.x, b.x, eps) && near(a.y, b.y, eps); }

void testVec2()
{
    const Vec2 a{3, 4};
    CHECK(a.length() == 5.0f);
    CHECK(a + Vec2(1, 1) == Vec2(4, 5));
    CHECK(a - Vec2(1, 1) == Vec2(2, 3));
    CHECK(a * 2.0f == Vec2(6, 8));
    CHECK(2.0f * a == Vec2(6, 8));
    CHECK(a.dot({1, 0}) == 3.0f);
    CHECK(near(distance({0, 0}, {3, 4}), 5.0f));

    CHECK(near(a.withLength(10.0f), {6, 8}));
    CHECK(Vec2().withLength(5.0f).isNull()); // QLineF::setLength on a null line keeps it null
    CHECK(near(a.normalized().length(), 1.0f));

    // QLineF::angle() convention: screen coordinates (y down), counter-clockwise, [0, 360)
    CHECK(near(Vec2(1, 0).angleDeg(), 0.0f));
    CHECK(near(Vec2(0, -1).angleDeg(), 90.0f)); // up on screen
    CHECK(near(Vec2(-1, 0).angleDeg(), 180.0f));
    CHECK(near(Vec2(0, 1).angleDeg(), 270.0f)); // down on screen
    CHECK(near(Vec2(1, -1).angleDeg(), 45.0f));
    CHECK(Vec2().angleDeg() == 0.0f);

    // QLineF::setAngle(): keeps length
    CHECK(near(Vec2(2, 0).withAngleDeg(90.0f), {0, -2}));
    CHECK(near(Vec2(0, 5).withAngleDeg(180.0f), {-5, 0}));
    CHECK(near(Vec2::fromPolarDeg(270.0f, 3.0f), {0, 3}));

    // round trip over the full circle
    for (int deg = 0; deg < 360; deg += 15)
        CHECK(near(Vec2::fromPolarDeg(static_cast<float>(deg), 1.0f).angleDeg(), static_cast<float>(deg), 1e-3f));
}

void testRect()
{
    const Rect r{10, 20, 30, 40};
    CHECK(r.right() == 40.0f && r.bottom() == 60.0f);
    CHECK(r.center() == Vec2(25, 40));
    CHECK(Rect::fromCorners({10, 20}, {40, 60}) == r);

    // contains: edges inclusive
    CHECK(r.contains(Vec2(10, 20)));
    CHECK(r.contains(Vec2(40, 60)));
    CHECK(!r.contains(Vec2(40.01f, 30)));
    CHECK(r.contains(Rect(15, 25, 5, 5)));
    CHECK(!r.contains(Rect(15, 25, 50, 5)));

    // intersects: touching edges do not count
    CHECK(r.intersects(Rect(35, 55, 10, 10)));
    CHECK(!r.intersects(Rect(40, 20, 10, 10)));
    CHECK(!r.intersects(Rect(0, 0, 5, 5)));
    CHECK(!r.intersects(Rect()));

    CHECK(r.intersected(Rect(30, 50, 100, 100)) == Rect(30, 50, 10, 10));
    CHECK(r.intersected(Rect(100, 100, 1, 1)).isNull());
    CHECK(r.united(Rect(0, 0, 5, 5)) == Rect(0, 0, 40, 60));
    CHECK(Rect().united(r) == r);

    CHECK(r.translated(1, 2) == Rect(11, 22, 30, 40));
    CHECK(r.translated(Vec2(-10, -20)) == Rect(0, 0, 30, 40));
    Rect m = r;
    m.moveTo(0, 0);
    CHECK(m == Rect(0, 0, 30, 40));
    m.moveCenter({0, 0});
    CHECK(m == Rect(-15, -20, 30, 40));
    CHECK(r.inflated(5) == Rect(5, 15, 40, 50));

    CHECK(Rect(0, 0, 0, 5).isEmpty());
    CHECK(!Rect(0, 0, 0, 5).isNull());
    CHECK(Rect().isNull());
}

void testColor()
{
    CHECK(Color::fromRgb(0xffb439) == Color(0xff, 0xb4, 0x39, 255));
    CHECK(Color(9, 150, 9).withAlpha(100) == Color(9, 150, 9, 100));
    CHECK(Color().a == 255);
}

void testRandom()
{
    seedRandom(42);
    bool sawMin = false, sawMax = false;
    for (int i = 0; i < 1000; ++i) {
        const int v = randomInt(0, 45);
        CHECK(v >= 0 && v <= 45);
        sawMin |= v == 0;
        sawMax |= v == 45;
        const float f = randomFloat(-1.0f, 2.0f);
        CHECK(f >= -1.0f && f < 2.0f);
    }
    CHECK(sawMin && sawMax); // both bounds inclusive
    CHECK(randomInt(5, 5) == 5);
    CHECK(randomInt(5, 1) == 5);

    // same seed, same sequence
    seedRandom(7);
    const int first = randomInt(0, 1000000);
    seedRandom(7);
    CHECK(randomInt(0, 1000000) == first);
}

void testStrings()
{
    CHECK(format("level %d: %s (%.1f)", 3, "meadow", 2.5) == "level 3: meadow (2.5)");
    CHECK(format("%s", "").empty());
    const std::string longText(500, 'x');
    CHECK(format("%s!", longText.c_str()).size() == 501);

    const auto parts = split("a,b,,c", ',');
    CHECK(parts.size() == 4 && parts[0] == "a" && parts[2].empty() && parts[3] == "c");
    CHECK(split("a,b,,c", ',', true).size() == 3);
    CHECK(split("", ',').size() == 1);
    CHECK(split("", ',', true).empty());

    CHECK(trim("  hi there \t\n") == "hi there");
    CHECK(trim("   ").empty());
    CHECK(toLower("SheepDog") == "sheepdog");

    CHECK(parseInt(" 42 ") == 42);
    CHECK(parseInt("-7") == -7);
    CHECK(!parseInt("4x").has_value());
    CHECK(!parseInt("").has_value());
    CHECK(!parseInt("99999999999999").has_value());
    CHECK(parseFloat("1.5") == 1.5f);
    CHECK(!parseFloat("abc").has_value());
}

} // namespace

int main()
{
    testVec2();
    testRect();
    testColor();
    testRandom();
    testStrings();

    if (gFailures == 0)
        std::printf("all core tests passed\n");
    return gFailures == 0 ? 0 : 1;
}
