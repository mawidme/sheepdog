#include "level_data.hpp"

#include "core/strings.hpp"

#include <cJSON.h>

#include <cmath>
#include <memory>

using core::Vec2;

namespace game {

namespace {

using JsonPtr = std::unique_ptr<cJSON, decltype(&cJSON_Delete)>;

// Reads typed values from the JSON tree. The first problem is recorded in `error`;
// later reads still return defaults, so the parse code needs no error checks per field.
// (No exceptions: they are disabled in the Emscripten build.)
struct Reader {
    std::string error;

    void fail(std::string message)
    {
        if (error.empty())
            error = std::move(message);
    }

    const cJSON* member(const cJSON* obj, const char* key, const std::string& context)
    {
        const cJSON* item = obj ? cJSON_GetObjectItemCaseSensitive(obj, key) : nullptr;
        if (!item)
            fail(core::format("%s: missing \"%s\"", context.c_str(), key));
        return item;
    }

    int toInt(const cJSON* item, const std::string& context)
    {
        if (!item)
            return 0;
        if (!cJSON_IsNumber(item)) {
            fail(context + ": expected a number");
            return 0;
        }
        const double v = item->valuedouble;
        if (v != std::floor(v) || v < -1e9 || v > 1e9) {
            fail(context + ": expected an integer");
            return 0;
        }
        return static_cast<int>(v);
    }

    int intMember(const cJSON* obj, const char* key, const std::string& context)
    {
        return toInt(member(obj, key, context), context + "." + key);
    }

    // [x, y]
    Vec2 toPoint(const cJSON* item, const std::string& context)
    {
        if (!item)
            return {};
        if (!cJSON_IsArray(item) || cJSON_GetArraySize(item) != 2) {
            fail(context + ": expected [x, y]");
            return {};
        }
        return {static_cast<float>(toInt(cJSON_GetArrayItem(item, 0), context)),
                static_cast<float>(toInt(cJSON_GetArrayItem(item, 1), context))};
    }

    Vec2 pointMember(const cJSON* obj, const char* key, const std::string& context)
    {
        return toPoint(member(obj, key, context), context + "." + key);
    }

    // Calls fn(element, "key[i]") for each element of an (optionally required) array member.
    template <typename Fn>
    void forEachIn(const cJSON* root, const char* key, bool required, Fn fn)
    {
        const cJSON* arr = cJSON_GetObjectItemCaseSensitive(root, key);
        if (!arr) {
            if (required)
                fail(core::format("missing \"%s\"", key));
            return;
        }
        if (!cJSON_IsArray(arr)) {
            fail(core::format("\"%s\" must be an array", key));
            return;
        }
        int i = 0;
        const cJSON* el = nullptr;
        cJSON_ArrayForEach(el, arr) {
            fn(el, core::format("%s[%d]", key, i++));
        }
    }

    std::vector<PropData> props(const cJSON* root, const char* key)
    {
        std::vector<PropData> result;
        forEachIn(root, key, false, [&](const cJSON* el, const std::string& ctx) {
            result.push_back({pointMember(el, "pos", ctx), intMember(el, "style", ctx)});
        });
        return result;
    }
};

// --- writing ----------------------------------------------------------------

cJSON* point(Vec2 p)
{
    const int xy[2] = {static_cast<int>(std::lround(p.x)), static_cast<int>(std::lround(p.y))};
    return cJSON_CreateIntArray(xy, 2);
}

void addProps(cJSON* root, const char* key, const std::vector<PropData>& props)
{
    if (props.empty())
        return;
    cJSON* arr = cJSON_AddArrayToObject(root, key);
    for (const PropData& p : props) {
        cJSON* o = cJSON_CreateObject();
        cJSON_AddItemToObject(o, "pos", point(p.pos));
        cJSON_AddNumberToObject(o, "style", p.style);
        cJSON_AddItemToArray(arr, o);
    }
}

} // namespace

std::optional<LevelData> parseLevel(std::string_view json, std::string* error)
{
    JsonPtr root(cJSON_ParseWithLength(json.data(), json.size()), &cJSON_Delete);
    if (!root || !cJSON_IsObject(root.get())) {
        if (error) {
            const char* where = cJSON_GetErrorPtr();
            *error = where ? core::format("invalid JSON near offset %d",
                                          static_cast<int>(where - json.data()))
                           : "invalid JSON";
        }
        return std::nullopt;
    }

    Reader in;
    const cJSON* r = root.get();
    LevelData level;

    const int version = in.intMember(r, "version", "level");
    if (in.error.empty() && version != kLevelFormatVersion)
        in.fail(core::format("unsupported level format version %d", version));

    const Vec2 size = in.pointMember(r, "size", "level");
    level.width = static_cast<int>(size.x);
    level.height = static_cast<int>(size.y);
    if (in.error.empty() && (level.width <= 0 || level.height <= 0))
        in.fail("level.size must be positive");
    level.timeSeconds = in.intMember(r, "time", "level");

    in.forEachIn(r, "fences", false, [&](const cJSON* el, const std::string& ctx) {
        level.fences.push_back({in.pointMember(el, "from", ctx), in.pointMember(el, "to", ctx)});
    });
    in.forEachIn(r, "water", false, [&](const cJSON* el, const std::string& ctx) {
        level.water.push_back(
            {in.pointMember(el, "from", ctx), in.pointMember(el, "to", ctx), in.intMember(el, "style", ctx)});
    });
    level.bushes = in.props(r, "bushes");
    level.trees = in.props(r, "trees");
    level.exits = in.props(r, "exits");
    in.forEachIn(r, "dogs", true, [&](const cJSON* el, const std::string& ctx) {
        level.dogs.push_back(in.toPoint(el, ctx));
    });
    in.forEachIn(r, "sheep", true, [&](const cJSON* el, const std::string& ctx) {
        level.sheep.push_back(in.toPoint(el, ctx));
    });

    if (!in.error.empty()) {
        if (error)
            *error = in.error;
        return std::nullopt;
    }
    return level;
}

std::string serializeLevel(const LevelData& level)
{
    JsonPtr root(cJSON_CreateObject(), &cJSON_Delete);
    cJSON* r = root.get();
    cJSON_AddNumberToObject(r, "version", kLevelFormatVersion);
    cJSON_AddItemToObject(r, "size", point({static_cast<float>(level.width), static_cast<float>(level.height)}));
    cJSON_AddNumberToObject(r, "time", level.timeSeconds);

    if (!level.fences.empty()) {
        cJSON* arr = cJSON_AddArrayToObject(r, "fences");
        for (const FenceData& f : level.fences) {
            cJSON* o = cJSON_CreateObject();
            cJSON_AddItemToObject(o, "from", point(f.from));
            cJSON_AddItemToObject(o, "to", point(f.to));
            cJSON_AddItemToArray(arr, o);
        }
    }
    if (!level.water.empty()) {
        cJSON* arr = cJSON_AddArrayToObject(r, "water");
        for (const WaterData& w : level.water) {
            cJSON* o = cJSON_CreateObject();
            cJSON_AddItemToObject(o, "from", point(w.from));
            cJSON_AddItemToObject(o, "to", point(w.to));
            cJSON_AddNumberToObject(o, "style", w.style);
            cJSON_AddItemToArray(arr, o);
        }
    }
    addProps(r, "bushes", level.bushes);
    addProps(r, "trees", level.trees);
    addProps(r, "exits", level.exits);

    cJSON* dogs = cJSON_AddArrayToObject(r, "dogs");
    for (Vec2 d : level.dogs)
        cJSON_AddItemToArray(dogs, point(d));
    cJSON* sheep = cJSON_AddArrayToObject(r, "sheep");
    for (Vec2 s : level.sheep)
        cJSON_AddItemToArray(sheep, point(s));

    std::unique_ptr<char, decltype(&cJSON_free)> text(cJSON_Print(r), &cJSON_free);
    return text ? std::string(text.get()) + "\n" : std::string();
}

} // namespace game
