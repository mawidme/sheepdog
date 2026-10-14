// Unit tests for the SDL-free parts of src/gfx (camera maths, shape tessellation).
#include "core/math.hpp"
#include "gfx/camera.hpp"
#include "gfx/tessellate.hpp"

#include <cmath>
#include <cstdio>
#include <vector>

using core::Rect;
using core::Vec2;

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
bool relNear(float a, float b, float rel) { return std::fabs(a - b) <= rel * std::fabs(b); }

// Sum of triangle areas. Equals the shape's area only if triangles neither overlap nor leave gaps.
float area(const std::vector<Vec2>& tris)
{
    float sum = 0.0f;
    for (size_t i = 0; i + 2 < tris.size(); i += 3) {
        const Vec2 a = tris[i], b = tris[i + 1], c = tris[i + 2];
        sum += std::fabs((b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y)) * 0.5f;
    }
    return sum;
}

void testCamera()
{
    gfx::Camera cam;
    cam.viewportSize = {800, 480};
    cam.position = {100, 50};
    cam.zoom = 2.0f;

    CHECK(near(cam.worldToScreen(Vec2(100, 50)), {0, 0}));
    CHECK(near(cam.worldToScreen(Vec2(110, 60)), {20, 20}));
    CHECK(near(cam.screenToWorld(cam.worldToScreen(Vec2(321, 123))), {321, 123}));
    CHECK(cam.worldToScreen(Rect(110, 60, 10, 5)) == Rect(20, 20, 20, 10));
    CHECK(cam.visibleWorldRect() == Rect(100, 50, 400, 240));

    // zooming keeps the world point under the cursor in place
    const Vec2 cursor{300, 200};
    const Vec2 before = cam.screenToWorld(cursor);
    cam.zoomAt(cursor, 1.5f);
    CHECK(near(cam.zoom, 3.0f));
    CHECK(near(cam.screenToWorld(cursor), before));

    // zoom is clamped, anchor still holds
    cam.zoomAt(cursor, 10.0f);
    CHECK(near(cam.zoom, cam.maxZoom));
    CHECK(near(cam.screenToWorld(cursor), before));
    cam.zoomAt(cursor, 0.001f);
    CHECK(near(cam.zoom, cam.minZoom));

    // panning: dragging right moves the view left in the world
    cam.zoom = 2.0f;
    cam.position = {100, 100};
    cam.panByScreen({20, -10});
    CHECK(near(cam.position, {90, 105}));

    // clamping to a level larger than the view
    const Rect level{0, 0, 1600, 960};
    cam.zoom = 1.0f;
    cam.position = {-50, 700};
    cam.clampTo(level);
    CHECK(near(cam.position, {0, 480}));
    cam.position = {5000, -5};
    cam.clampTo(level);
    CHECK(near(cam.position, {800, 0}));

    // level smaller than the view: centred
    cam.zoom = 0.4f; // view is 2000 x 1200 world units
    cam.clampTo(level);
    CHECK(near(cam.position, {-200, -120}));
}

void testTessellation()
{
    using namespace gfx::tess;
    std::vector<Vec2> t;

    rect(t, {10, 10, 30, 20});
    CHECK(t.size() == 6);
    CHECK(near(area(t), 600.0f));

    t.clear();
    rectOutline(t, {0, 0, 100, 50}, 4.0f); // stroke centred: outer 104x54, inner 96x46
    CHECK(t.size() % 3 == 0);
    CHECK(near(area(t), 104.0f * 54.0f - 96.0f * 46.0f));

    t.clear();
    ellipse(t, {0, 0, 100, 60}, 128);
    CHECK(t.size() == 128 * 3);
    CHECK(relNear(area(t), core::kPi * 50.0f * 30.0f, 0.005f));

    t.clear();
    ellipseOutline(t, {0, 0, 100, 100}, 10.0f, 128); // ring between r=45 and r=55
    CHECK(relNear(area(t), core::kPi * (55.0f * 55.0f - 45.0f * 45.0f), 0.005f));

    t.clear();
    roundedRect(t, {0, 0, 200, 100}, 20.0f, 32);
    CHECK(relNear(area(t), 200.0f * 100.0f - (4.0f - core::kPi) * 400.0f, 0.002f));

    t.clear();
    roundedRect(t, {0, 0, 40, 20}, 50.0f, 32); // radius clamped to 10: a stadium shape
    CHECK(relNear(area(t), 20.0f * 20.0f + core::kPi * 100.0f, 0.005f));

    t.clear();
    roundedRect(t, {0, 0, 40, 20}, 0.0f, 8); // plain rect
    CHECK(t.size() == 6);

    t.clear();
    line(t, {0, 0}, {30, 40}, 4.0f);
    CHECK(near(area(t), 50.0f * 4.0f));
    t.clear();
    line(t, {5, 5}, {5, 5}, 4.0f); // degenerate: nothing
    CHECK(t.empty());

    CHECK(circleSegments(0.0f) == 12);
    CHECK(circleSegments(10000.0f) == 128);
    CHECK(circleSegments(50.0f) > circleSegments(10.0f));
}

} // namespace

int main()
{
    testCamera();
    testTessellation();

    if (gFailures == 0)
        std::printf("all gfx tests passed\n");
    return gFailures == 0 ? 0 : 1;
}
