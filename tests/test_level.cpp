// Tests for the JSON level format: the converted built-in levels, round trips and error handling.
#include "game/level_data.hpp"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

using core::Vec2;
using game::LevelData;
using game::parseLevel;
using game::serializeLevel;

namespace {

int gFailures = 0;

#define CHECK(cond)                                                              \
    do {                                                                         \
        if (!(cond)) {                                                           \
            std::printf("%s:%d: CHECK failed: %s\n", __FILE__, __LINE__, #cond); \
            ++gFailures;                                                         \
        }                                                                        \
    } while (0)

std::string readFile(const std::string& path)
{
    std::ifstream in(path, std::ios::binary);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

std::optional<LevelData> loadBuiltIn(int number)
{
    const std::string path = std::string(SHEEPDOG_ASSETS_DIR) + "/levels/level" + std::to_string(number) + ".json";
    std::string error;
    auto level = parseLevel(readFile(path), &error);
    if (!level)
        std::printf("%s: %s\n", path.c_str(), error.c_str());
    return level;
}

// element counts from the original XML files
struct Expected {
    int width, height, time, fences, water, bushes, trees, exits, dogs, sheep;
};

void testBuiltInLevels()
{
    const Expected expected[] = {
        {503, 420, 120, 36, 0, 0, 0, 4, 1, 8},
        {364, 234, 120, 8, 0, 7, 3, 1, 1, 1},
        {0, 0, 0, 11, 0, 9, 8, 2, 1, 2},
        {0, 0, 0, 18, 0, 9, 5, 3, 1, 6},
    };
    for (int i = 0; i < 4; ++i) {
        const auto level = loadBuiltIn(i);
        CHECK(level.has_value());
        if (!level)
            continue;
        const Expected& e = expected[i];
        if (e.width != 0) {
            CHECK(level->width == e.width);
            CHECK(level->height == e.height);
            CHECK(level->timeSeconds == e.time);
        }
        CHECK(level->width > 0 && level->height > 0 && level->timeSeconds > 0);
        CHECK(static_cast<int>(level->fences.size()) == e.fences);
        CHECK(static_cast<int>(level->water.size()) == e.water);
        CHECK(static_cast<int>(level->bushes.size()) == e.bushes);
        CHECK(static_cast<int>(level->trees.size()) == e.trees);
        CHECK(static_cast<int>(level->exits.size()) == e.exits);
        CHECK(static_cast<int>(level->dogs.size()) == e.dogs);
        CHECK(static_cast<int>(level->sheep.size()) == e.sheep);

        // what we write, we read back identically
        const auto again = parseLevel(serializeLevel(*level));
        CHECK(again.has_value() && *again == *level);
    }

    // spot checks against level0.xml
    const auto level0 = loadBuiltIn(0);
    if (level0) {
        CHECK(level0->fences.front().from == Vec2(20, 7) && level0->fences.front().to == Vec2(107, 7));
        CHECK(level0->exits.back().pos == Vec2(477, 204) && level0->exits.back().style == 1);
        CHECK(level0->dogs.front() == Vec2(203, 120));
        CHECK(level0->sheep.back() == Vec2(247, 97));
    }
}

void testRoundTripAllElements()
{
    LevelData level;
    level.width = 800;
    level.height = 600;
    level.timeSeconds = 90;
    level.fences = {{{10, 10}, {200, 10}}};
    level.water = {{{300, 300}, {400, 380}, 2}};
    level.bushes = {{{50, 60}, 1}};
    level.trees = {{{70, 80}, 1}};
    level.exits = {{{0, 100}, 1}};
    level.dogs = {{400, 300}};
    level.sheep = {{100, 100}, {120, 110}};

    const auto parsed = parseLevel(serializeLevel(level));
    CHECK(parsed.has_value() && *parsed == level);

    // positions are stored as integers
    level.sheep[0] = {100.4f, 99.6f};
    const auto rounded = parseLevel(serializeLevel(level));
    CHECK(rounded.has_value() && rounded->sheep[0] == Vec2(100, 100));
}

bool fails(const char* json, const char* expectedErrorPart)
{
    std::string error;
    const bool failed = !parseLevel(json, &error).has_value();
    if (failed && error.find(expectedErrorPart) == std::string::npos)
        std::printf("unexpected error message: '%s' (expected '%s')\n", error.c_str(), expectedErrorPart);
    return failed && error.find(expectedErrorPart) != std::string::npos;
}

void testErrors()
{
    // minimal valid level: optional arrays omitted, characters may be empty
    CHECK(parseLevel(R"({"version": 1, "size": [10, 10], "time": 5, "dogs": [], "sheep": []})").has_value());

    CHECK(fails("", "invalid JSON"));
    CHECK(fails("{\"version\": 1,", "invalid JSON"));
    CHECK(fails("[]", "invalid JSON"));
    CHECK(fails(R"({"version": 2, "size": [10, 10], "time": 5, "dogs": [], "sheep": []})", "version 2"));
    CHECK(fails(R"({"size": [10, 10], "time": 5, "dogs": [], "sheep": []})", "\"version\""));
    CHECK(fails(R"({"version": 1, "size": [10], "time": 5, "dogs": [], "sheep": []})", "level.size"));
    CHECK(fails(R"({"version": 1, "size": [0, 10], "time": 5, "dogs": [], "sheep": []})", "positive"));
    CHECK(fails(R"({"version": 1, "size": [10, 10], "time": 5.5, "dogs": [], "sheep": []})", "integer"));
    CHECK(fails(R"({"version": 1, "size": [10, 10], "time": "5", "dogs": [], "sheep": []})", "number"));
    CHECK(fails(R"({"version": 1, "size": [10, 10], "time": 5, "dogs": []})", "\"sheep\""));
    CHECK(fails(R"({"version": 1, "size": [10, 10], "time": 5, "dogs": [], "sheep": [[1, 2, 3]]})", "sheep[0]"));
    CHECK(fails(R"({"version": 1, "size": [10, 10], "time": 5, "dogs": [], "sheep": [],
                    "fences": [{"from": [1, 1]}]})", "fences[0]: missing \"to\""));
    CHECK(fails(R"({"version": 1, "size": [10, 10], "time": 5, "dogs": [], "sheep": [],
                    "bushes": [{"pos": [1, 1]}]})", "bushes[0]: missing \"style\""));
    CHECK(fails(R"({"version": 1, "size": [10, 10], "time": 5, "dogs": [], "sheep": [], "trees": {}})", "array"));
}

} // namespace

int main()
{
    testBuiltInLevels();
    testRoundTripAllElements();
    testErrors();

    if (gFailures == 0)
        std::printf("all level tests passed\n");
    return gFailures == 0 ? 0 : 1;
}
