#pragma once

#include "core/rect.hpp"
#include "core/vec2.hpp"
#include "level_data.hpp"

#include <optional>
#include <string>
#include <vector>

// Level editor logic (Qt: the editing part of Level). Works directly on LevelData; the
// presentation layer maps toolbar clicks and pointer events to these calls.
namespace game {

enum class EditTool { Fence, Bush, Tree, Water, Exit, Dog, Sheep, Eraser, Pan };

inline constexpr int kDefaultEditorLevelTime = 120; // seconds (the Qt editor had no way to change it)

class LevelEditor {
public:
    LevelEditor();

    // start over with an empty level
    void clear();
    // edit an existing level (number of the user level it was loaded from, 0 = none)
    void load(const LevelData& level, int userLevelNumber);

    const LevelData& level() const { return m_level; }
    int userLevelNumber() const { return m_userLevelNumber; }
    void setUserLevelNumber(int n) { m_userLevelNumber = n; }

    EditTool tool = EditTool::Fence;

    // Pointer input in level coordinates. A press adds (or erases) an object, dragging shapes
    // or moves it, release finishes it.
    void press(core::Vec2 p);
    void drag(core::Vec2 p);
    void release();

    bool canUndo() const { return !m_undo.empty(); }
    void undo();

    // unsaved changes
    bool dirty() const { return m_dirty; }
    void markSaved() { m_dirty = false; }

    bool empty() const;
    // nullopt if playable, otherwise what is missing
    std::optional<std::string> validate() const;
    // Moves everything so the level starts at (0, 0) and sets its size from the content
    // (Qt: editPrepareLevel()). Returns the shift applied, so the view can follow.
    core::Vec2 normalize();
    // bounding box of all objects, as used by normalize()
    core::Rect contentBounds() const;

private:
    enum class Dragged { None, Fence, Water, Bush, Tree, Exit, Dog, Sheep };

    void pushUndo(const LevelData& before);
    core::Vec2 snapFence(core::Vec2 p, size_t ignore) const;
    bool erase(core::Vec2 p);

    LevelData m_level;
    int m_userLevelNumber = 0;
    bool m_dirty = false;
    bool m_dirtyBeforePress = false;
    std::vector<LevelData> m_undo;

    Dragged m_dragged = Dragged::None;
    size_t m_draggedIndex = 0;
    core::Vec2 m_waterStart;
    std::optional<LevelData> m_beforeWater; // state before a water drag added something
};

} // namespace game
