#include "level_editor.hpp"

#include "character.hpp"
#include "constants.hpp"
#include "element.hpp"

#include <algorithm>
#include <cmath>

using core::Rect;
using core::Vec2;

namespace game {

namespace {

constexpr float kFenceSnapDist = 20.0f; // Qt: searchDist in editAddFence()
constexpr float kEraseSearchDist = 10.0f;
constexpr size_t kMaxUndoSteps = 200;

// fences are horizontal or vertical: keep the major axis, take the other coordinate from the start
Vec2 axisAligned(Vec2 from, Vec2 p)
{
    return std::fabs(p.x - from.x) < std::fabs(p.y - from.y) ? Vec2(from.x, p.y) : Vec2(p.x, from.y);
}

Vec2 propSize(EditTool tool)
{
    switch (tool) {
    case EditTool::Bush:
        return kBushImageSize;
    case EditTool::Tree:
        return kTreeImageSize;
    default:
        return kExitImageSize;
    }
}

// area a character covers in the editor (Qt: square bounding rect around the centre)
Rect characterBounds(Vec2 mid, Vec2 imageSize)
{
    const float side = std::max(imageSize.x, imageSize.y);
    return {mid - Vec2(side, side) * 0.5f, {side, side}};
}

} // namespace

LevelEditor::LevelEditor()
{
    clear();
}

void LevelEditor::clear()
{
    m_level = LevelData();
    m_level.timeSeconds = kDefaultEditorLevelTime;
    m_userLevelNumber = 0;
    m_dirty = false;
    m_undo.clear();
    m_dragged = Dragged::None;
}

void LevelEditor::load(const LevelData& level, int userLevelNumber)
{
    clear();
    m_level = level;
    m_userLevelNumber = userLevelNumber;
}

bool LevelEditor::empty() const
{
    return m_level.fences.empty() && m_level.water.empty() && m_level.bushes.empty() && m_level.trees.empty() &&
           m_level.exits.empty() && m_level.dogs.empty() && m_level.sheep.empty();
}

void LevelEditor::pushUndo(const LevelData& before)
{
    m_undo.push_back(before);
    if (m_undo.size() > kMaxUndoSteps)
        m_undo.erase(m_undo.begin());
    m_dirty = true;
    m_pressAddedUndo = true;
}

void LevelEditor::undo()
{
    if (m_undo.empty())
        return;
    m_level = m_undo.back();
    m_undo.pop_back();
    m_dragged = Dragged::None;
    // undoing everything back to an empty level leaves nothing worth saving
    m_dirty = !empty();
}

Vec2 LevelEditor::snapFence(Vec2 p, size_t ignore) const
{
    // Connect to a fence nearby: to its start or end point if close, otherwise onto its line
    // (Qt: Level::editAddFence(); the Qt version snapped only the start).
    const Rect search{p - Vec2(kFenceSnapDist, kFenceSnapDist), {2 * kFenceSnapDist, 2 * kFenceSnapDist}};
    for (size_t i = 0; i < m_level.fences.size(); ++i) {
        if (i == ignore)
            continue;
        const FenceData& f = m_level.fences[i];
        const Element e = makeFence(f.from, f.to);
        if (!search.intersects(e.bounds))
            continue;
        if (distance(f.from, p) < kFenceSnapDist)
            return f.from;
        if (distance(f.to, p) < kFenceSnapDist)
            return f.to;
        return e.vertical ? Vec2(f.from.x, p.y) : Vec2(p.x, f.from.y);
    }
    return p;
}

bool LevelEditor::erase(Vec2 p)
{
    // first object whose collision area is near the point (Qt: Level::editErase())
    const Rect search{p - Vec2(kEraseSearchDist, kEraseSearchDist), {2 * kEraseSearchDist, 2 * kEraseSearchDist}};
    auto hits = [&](const std::optional<Rect>& r) { return r && search.intersects(*r); };

    for (size_t i = 0; i < m_level.fences.size(); ++i)
        if (hits(makeFence(m_level.fences[i].from, m_level.fences[i].to).collision)) {
            m_level.fences.erase(m_level.fences.begin() + static_cast<long>(i));
            return true;
        }
    for (size_t i = 0; i < m_level.water.size(); ++i) {
        const auto w = makeWater(m_level.water[i].from, m_level.water[i].to, 1);
        if (!w || hits(w->collision)) {
            m_level.water.erase(m_level.water.begin() + static_cast<long>(i));
            return true;
        }
    }
    auto eraseProp = [&](std::vector<PropData>& props, Element (*make)(Vec2, int)) {
        for (size_t i = 0; i < props.size(); ++i)
            if (hits(make(props[i].pos, props[i].style).collision)) {
                props.erase(props.begin() + static_cast<long>(i));
                return true;
            }
        return false;
    };
    if (eraseProp(m_level.bushes, makeBush) || eraseProp(m_level.trees, makeTree) || eraseProp(m_level.exits, makeExit))
        return true;
    for (size_t i = 0; i < m_level.dogs.size(); ++i)
        if (hits(Dog(m_level.dogs[i]).collisionRect())) {
            m_level.dogs.erase(m_level.dogs.begin() + static_cast<long>(i));
            return true;
        }
    for (size_t i = 0; i < m_level.sheep.size(); ++i)
        if (hits(Sheep(m_level.sheep[i], 0.0f, 0.0f, 0).collisionRect())) {
            m_level.sheep.erase(m_level.sheep.begin() + static_cast<long>(i));
            return true;
        }
    return false;
}

void LevelEditor::press(Vec2 p)
{
    const LevelData before = m_level;
    m_dirtyBeforePress = m_dirty;
    m_pressAddedUndo = false;
    m_dragged = Dragged::None;

    switch (tool) {
    case EditTool::Fence: {
        const Vec2 start = snapFence(p, m_level.fences.size());
        m_level.fences.push_back({start, start});
        m_dragged = Dragged::Fence;
        m_draggedIndex = m_level.fences.size() - 1;
        break;
    }
    case EditTool::Bush:
    case EditTool::Tree:
    case EditTool::Exit: {
        // placed centred on the pointer
        auto& props = tool == EditTool::Bush ? m_level.bushes : tool == EditTool::Tree ? m_level.trees : m_level.exits;
        props.push_back({p - propSize(tool) * 0.5f, 1});
        m_dragged = tool == EditTool::Bush ? Dragged::Bush : tool == EditTool::Tree ? Dragged::Tree : Dragged::Exit;
        m_draggedIndex = props.size() - 1;
        break;
    }
    case EditTool::Water:
        // nothing yet: water appears once dragged large enough
        m_waterStart = p;
        m_beforeWater = before;
        m_dragged = Dragged::Water;
        return;
    case EditTool::Dog:
        // only one dog: a new one replaces the old one
        m_level.dogs = {p};
        m_dragged = Dragged::Dog;
        m_draggedIndex = 0;
        break;
    case EditTool::Sheep:
        m_level.sheep.push_back(p);
        m_dragged = Dragged::Sheep;
        m_draggedIndex = m_level.sheep.size() - 1;
        break;
    case EditTool::Eraser:
        if (!erase(p))
            return;
        break;
    case EditTool::Pan:
        return; // handled by the view
    }
    pushUndo(before);
}

void LevelEditor::drag(Vec2 p)
{
    switch (m_dragged) {
    case Dragged::None:
        break;
    case Dragged::Fence: {
        FenceData& f = m_level.fences[m_draggedIndex];
        f.to = axisAligned(f.from, snapFence(p, m_draggedIndex));
        break;
    }
    case Dragged::Bush:
        m_level.bushes[m_draggedIndex].pos = p - kBushImageSize * 0.5f;
        break;
    case Dragged::Tree:
        m_level.trees[m_draggedIndex].pos = p - kTreeImageSize * 0.5f;
        break;
    case Dragged::Exit:
        m_level.exits[m_draggedIndex].pos = p - kExitImageSize * 0.5f;
        break;
    case Dragged::Dog:
        m_level.dogs[m_draggedIndex] = p;
        break;
    case Dragged::Sheep:
        m_level.sheep[m_draggedIndex] = p;
        break;
    case Dragged::Water: {
        const bool bigEnough = makeWater(m_waterStart, p, 1).has_value();
        const bool added = !m_beforeWater.has_value();
        if (bigEnough && !added) {
            pushUndo(*m_beforeWater);
            m_beforeWater.reset();
            m_level.water.push_back({m_waterStart, p, 1});
        } else if (bigEnough) {
            m_level.water.back().to = p;
        } else if (added) {
            // shrunk below the minimum size again: as if nothing happened
            m_level = m_undo.back();
            m_undo.pop_back();
            m_dirty = m_dirtyBeforePress;
            m_pressAddedUndo = false;
            m_beforeWater = m_level;
        }
        break;
    }
    }
}

void LevelEditor::release()
{
    // a click without dragging leaves no (zero length) fence behind
    if (m_dragged == Dragged::Fence) {
        const FenceData& f = m_level.fences[m_draggedIndex];
        if (distance(f.from, f.to) < kFenceWidth) {
            m_level = m_undo.back();
            m_undo.pop_back();
            m_dirty = m_dirtyBeforePress;
        }
    }
    m_dragged = Dragged::None;
    m_beforeWater.reset();
    m_pressAddedUndo = false;
}

void LevelEditor::cancel()
{
    if (m_pressAddedUndo) {
        m_level = m_undo.back();
        m_undo.pop_back();
        m_dirty = m_dirtyBeforePress;
    }
    m_dragged = Dragged::None;
    m_beforeWater.reset();
    m_pressAddedUndo = false;
}

std::optional<std::string> LevelEditor::validate() const
{
    // same messages as Level::checkLevel()
    if (m_level.dogs.empty())
        return std::string("add at least one dog\nto the level!");
    if (m_level.sheep.empty())
        return std::string("add at least one sheep\nto the level!");
    if (m_level.exits.empty())
        return std::string("add at least one exit\nto the level!");
    return std::nullopt;
}

Rect LevelEditor::contentBounds() const
{
    Rect bounds;
    auto add = [&](const Rect& r) { bounds = bounds.isNull() ? r : bounds.united(r); };
    for (const FenceData& f : m_level.fences)
        add(makeFence(f.from, f.to).bounds);
    for (const WaterData& w : m_level.water)
        if (auto e = makeWater(w.from, w.to, w.style))
            add(e->bounds);
    for (const PropData& b : m_level.bushes)
        add(makeBush(b.pos, b.style).bounds);
    for (const PropData& t : m_level.trees)
        add(makeTree(t.pos, t.style).bounds);
    for (const PropData& e : m_level.exits)
        add(makeExit(e.pos, e.style).bounds);
    for (Vec2 d : m_level.dogs)
        add(characterBounds(d, kDogImageSize));
    for (Vec2 s : m_level.sheep)
        add(characterBounds(s, kSheepImageSize));
    return bounds;
}

Vec2 LevelEditor::normalize()
{
    const Rect bounds = contentBounds();
    if (bounds.isNull())
        return {};
    // whole units, as levels are saved with integer coordinates
    const Vec2 shift{-std::floor(bounds.x), -std::floor(bounds.y)};
    for (FenceData& f : m_level.fences) {
        f.from += shift;
        f.to += shift;
    }
    for (WaterData& w : m_level.water) {
        w.from += shift;
        w.to += shift;
    }
    for (auto* props : {&m_level.bushes, &m_level.trees, &m_level.exits})
        for (PropData& p : *props)
            p.pos += shift;
    for (Vec2& d : m_level.dogs)
        d += shift;
    for (Vec2& s : m_level.sheep)
        s += shift;
    m_level.width = static_cast<int>(std::ceil(bounds.right() + shift.x));
    m_level.height = static_cast<int>(std::ceil(bounds.bottom() + shift.y));
    if (m_level.timeSeconds <= 0)
        m_level.timeSeconds = kDefaultEditorLevelTime;
    return shift;
}

} // namespace game
