// Tests for the level editor logic (src/game/level_editor).
#include "game/constants.hpp"
#include "game/level_editor.hpp"
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
bool near(Vec2 a, Vec2 b, float eps = 1e-3f) { return near(a.x, b.x, eps) && near(a.y, b.y, eps); }

void place(LevelEditor& e, EditTool tool, Vec2 p)
{
    e.tool = tool;
    e.press(p);
    e.release();
}

void drawFence(LevelEditor& e, Vec2 from, Vec2 to)
{
    e.tool = EditTool::Fence;
    e.press(from);
    e.drag(to);
    e.release();
}

void testPlacingObjects()
{
    LevelEditor e;
    CHECK(e.empty() && !e.dirty() && !e.canUndo());

    place(e, EditTool::Bush, {100, 100});
    place(e, EditTool::Tree, {300, 100});
    place(e, EditTool::Exit, {500, 100});
    place(e, EditTool::Sheep, {100, 300});
    place(e, EditTool::Sheep, {200, 300});
    const LevelData& l = e.level();
    CHECK(l.bushes.size() == 1 && l.trees.size() == 1 && l.exits.size() == 1 && l.sheep.size() == 2);
    // props are centred on the click
    CHECK(near(l.bushes[0].pos + kBushImageSize * 0.5f, {100, 100}));
    CHECK(near(l.exits[0].pos + kExitImageSize * 0.5f, {500, 100}));
    CHECK(e.dirty() && e.canUndo());

    // only one dog: a second one replaces the first
    place(e, EditTool::Dog, {400, 400});
    place(e, EditTool::Dog, {450, 400});
    CHECK(l.dogs.size() == 1 && l.dogs[0] == Vec2(450, 400));

    // dragging moves what was just placed
    e.tool = EditTool::Sheep;
    e.press({600, 600});
    e.drag({650, 620});
    e.release();
    CHECK(l.sheep.back() == Vec2(650, 620));
}

void testFences()
{
    LevelEditor e;
    // fences are axis aligned along the larger movement
    drawFence(e, {100, 100}, {300, 110});
    CHECK(e.level().fences.size() == 1);
    CHECK(e.level().fences[0].to == Vec2(300, 100));
    drawFence(e, {500, 100}, {505, 300});
    CHECK(e.level().fences[1].to == Vec2(500, 300));

    // a click without dragging leaves nothing, and isn't an unsaved change
    LevelEditor e2;
    e2.tool = EditTool::Fence;
    e2.press({50, 50});
    e2.release();
    CHECK(e2.level().fences.empty() && !e2.dirty() && !e2.canUndo());

    // a new fence starting near an end connects to it
    drawFence(e, {310, 92}, {310, 250});
    const FenceData& f = e.level().fences[2];
    CHECK(f.from == Vec2(300, 100));
    CHECK(f.to == Vec2(300, 250)); // axis aligned from the snapped start

    // starting near the middle of a fence: onto its line
    drawFence(e, {200, 112}, {200, 300});
    CHECK(e.level().fences[3].from == Vec2(200, 100));

    // the end snaps onto another fence, e.g. to close a pen
    drawFence(e, {300, 250}, {492, 255});
    CHECK(e.level().fences[4].to == Vec2(500, 250));
    // ... or to its end point
    drawFence(e, {300, 300}, {488, 296});
    CHECK(e.level().fences[5].to == Vec2(500, 300));
}

void testWater()
{
    LevelEditor e;
    e.tool = EditTool::Water;
    e.press({100, 100});
    e.drag({110, 110}); // too small: nothing yet
    CHECK(e.level().water.empty() && !e.dirty());
    e.drag({200, 180});
    CHECK(e.level().water.size() == 1 && e.level().water[0].to == Vec2(200, 180));
    e.drag({105, 180}); // too narrow again: removed
    CHECK(e.level().water.empty() && !e.canUndo());
    e.drag({250, 250});
    e.release();
    CHECK(e.level().water.size() == 1 && e.level().water[0].to == Vec2(250, 250));
    e.undo();
    CHECK(e.level().water.empty() && !e.canUndo());
}

void testEraseAndUndo()
{
    LevelEditor e;
    drawFence(e, {100, 100}, {300, 100});
    place(e, EditTool::Bush, {100, 300});
    place(e, EditTool::Sheep, {400, 300});
    place(e, EditTool::Dog, {500, 300});

    place(e, EditTool::Eraser, {700, 700}); // nothing there: no change, no undo step
    CHECK(e.level().fences.size() == 1);

    place(e, EditTool::Eraser, {200, 100}); // on the fence
    CHECK(e.level().fences.empty());
    place(e, EditTool::Eraser, {400, 300});
    CHECK(e.level().sheep.empty());
    place(e, EditTool::Eraser, {100, 300});
    CHECK(e.level().bushes.empty());
    CHECK(e.level().dogs.size() == 1);

    // undo brings erased objects back too, one step at a time
    e.undo();
    CHECK(e.level().bushes.size() == 1);
    e.undo();
    e.undo();
    CHECK(e.level().fences.size() == 1 && e.level().sheep.size() == 1);
    while (e.canUndo())
        e.undo();
    CHECK(e.empty() && !e.dirty());
}

void testValidateAndNormalize()
{
    LevelEditor e;
    CHECK(e.validate()->find("dog") != std::string::npos);
    place(e, EditTool::Dog, {-50, 20});
    CHECK(e.validate()->find("sheep") != std::string::npos);
    place(e, EditTool::Sheep, {100, 100});
    CHECK(e.validate()->find("exit") != std::string::npos);
    place(e, EditTool::Exit, {300, -100});
    CHECK(!e.validate().has_value());

    // everything moves so the level starts at (0, 0); the size follows the content
    const Rect before = e.contentBounds();
    const Vec2 dogBefore = e.level().dogs[0];
    const Vec2 shift = e.normalize();
    CHECK(shift.x == -std::floor(before.x) && shift.y == -std::floor(before.y));
    CHECK(e.level().dogs[0] == dogBefore + shift);
    const Rect after = e.contentBounds();
    // (float rounding: the left edge may end up a few millionths below 0)
    CHECK(after.x > -0.01f && after.x < 1.0f && after.y > -0.01f && after.y < 1.0f);
    CHECK(static_cast<float>(e.level().width) > after.right() - 0.01f &&
          static_cast<float>(e.level().width) < after.right() + 1.0f);
    CHECK(e.level().timeSeconds == kDefaultEditorLevelTime);

    // the result is a valid, playable level that survives saving and loading
    const auto reloaded = parseLevel(serializeLevel(e.level()));
    CHECK(reloaded.has_value());
    if (reloaded) {
        LevelPlay play(*reloaded, 0.0f);
        play.tick();
        CHECK(play.status() == LevelStatus::Playing);
    }
}

void testLoad()
{
    LevelData l;
    l.width = 400;
    l.height = 300;
    l.timeSeconds = 90;
    l.dogs = {{10, 10}};
    LevelEditor e;
    place(e, EditTool::Sheep, {1, 1});
    e.load(l, 3);
    CHECK(e.level() == l && e.userLevelNumber() == 3 && !e.dirty() && !e.canUndo());
    e.clear();
    CHECK(e.empty() && e.userLevelNumber() == 0 && e.level().timeSeconds == kDefaultEditorLevelTime);
}

} // namespace

int main()
{
    testPlacingObjects();
    testFences();
    testWater();
    testEraseAndUndo();
    testValidateAndNormalize();
    testLoad();

    if (gFailures == 0)
        std::printf("all level editor tests passed\n");
    return gFailures == 0 ? 0 : 1;
}
