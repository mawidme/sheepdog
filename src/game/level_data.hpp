#pragma once

#include "core/vec2.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

// Level description as stored in assets/levels/levelN.json (and user levels).
// Pure data; the game builds its objects from it. Replaces the XML level files.
//
// JSON format (version 1), coordinates in level units, all values integers:
//   {
//     "version": 1,
//     "size":    [width, height],
//     "time":    seconds,
//     "fences":  [{"from": [x, y], "to": [x, y]}, ...],          pole centres
//     "water":   [{"from": [x, y], "to": [x, y], "style": n}, ...], opposite corners
//     "bushes":  [{"pos": [x, y], "style": n}, ...],             top-left corner
//     "trees":   [{"pos": [x, y], "style": n}, ...],             top-left corner
//     "exits":   [{"pos": [x, y], "style": n}, ...],             top-left corner
//     "dogs":    [[x, y], ...],                                  centre
//     "sheep":   [[x, y], ...]                                   centre
//   }
// "fences", "water", "bushes", "trees", "exits" may be omitted when empty.
// Styles start at 1 (image variant, e.g. bush1.svg).
namespace game {

struct FenceData {
    core::Vec2 from;
    core::Vec2 to;
    bool operator==(const FenceData&) const = default;
};

struct WaterData {
    core::Vec2 from;
    core::Vec2 to;
    int style = 1;
    bool operator==(const WaterData&) const = default;
};

// bush, tree or exit
struct PropData {
    core::Vec2 pos;
    int style = 1;
    bool operator==(const PropData&) const = default;
};

struct LevelData {
    int width = 0;
    int height = 0;
    int timeSeconds = 0;
    std::vector<FenceData> fences;
    std::vector<WaterData> water;
    std::vector<PropData> bushes;
    std::vector<PropData> trees;
    std::vector<PropData> exits;
    std::vector<core::Vec2> dogs;
    std::vector<core::Vec2> sheep;

    bool operator==(const LevelData&) const = default;
};

inline constexpr int kLevelFormatVersion = 1;

// nullopt on error, with a human readable reason in *error (if given)
std::optional<LevelData> parseLevel(std::string_view json, std::string* error = nullptr);

// pretty-printed JSON; positions are rounded to integers
std::string serializeLevel(const LevelData& level);

} // namespace game
