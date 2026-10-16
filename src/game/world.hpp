#pragma once

#include "character.hpp"
#include "constants.hpp"
#include "element.hpp"
#include "level_data.hpp"

#include <vector>

// The simulated level: elements, characters, bark waves. One tick() is one frame of the Qt
// version (1/kTicksPerSecond). Level rules (time limit, winning, bonuses) build on top of this.
namespace game {

// Things the presentation layer reacts to (sounds), collected per tick.
enum class GameEvent {
    Bark,
    NoBark,              // not enough bark power
    BarkPowerCritical,   // only one more bark possible
    BarkPowerFull,
    SheepBaa,
    SheepRescued,
    // level rules (LevelPlay)
    BonusPickedUp,
    BonusEnded,
    TimeUp,
    LevelWon,
};

struct BarkWave {
    core::Vec2 center;
    float radius;
    float maxRadius;
    float growthPerTick;
};

class World {
public:
    explicit World(const LevelData& level);

    // One tick = beginTick() + advanceCharacters(); level rules run in between (Qt: Level::update()).
    void tick();
    void beginTick();
    void advanceCharacters();

    // Makes dog number `dog` bark; returns false if it couldn't (no bark power, no sheep).
    bool bark(size_t dog = 0);
    // Touch / mouse control: walk towards the point (Qt: MyGraphicsScene::moveDog()).
    void walkTo(size_t dog, core::Vec2 target);

    Tuning tuning;

    core::Vec2 size() const { return m_size; }
    long tickCount() const { return m_tick; }

    const std::vector<Element>& elements() const { return m_elements; }
    std::vector<Dog>& dogs() { return m_dogs; }
    const std::vector<Dog>& dogs() const { return m_dogs; }
    std::vector<Sheep>& sheep() { return m_sheep; }
    const std::vector<Sheep>& sheep() const { return m_sheep; }
    const std::vector<BarkWave>& barkWaves() const { return m_barkWaves; }

    int sheepTotal() const { return static_cast<int>(m_sheep.size()); }
    int sheepLeft() const { return m_sheepLeft; }

    // events since the last call
    std::vector<GameEvent> takeEvents();
    void pushEvent(GameEvent e) { m_events.push_back(e); }

    // spatial interval for collision checks (gColCheckStepDiff)
    float collisionStep() const { return m_colCheckStep; }

private:
    void advanceDog(Dog& dog);
    void advanceSheep(Sheep& sheep);
    void updateBarkWaves();

    // Moves c by its speed if nothing is in the way; otherwise walks up to the obstacle and
    // reacts (dog stops, sheep turns, sheep at an exit is rescued). Returns true on collision.
    bool checkCollisions(Character& c);
    // true if r collides with an element or a character of the other kind
    bool collidesAt(const Character& c, const core::Rect& r) const;
    void rescue(Sheep& sheep);
    void registerCharacter(const Character& c);

    core::Vec2 m_size;
    long m_tick = 0;
    float m_colCheckStep = 10000.0f;
    int m_sheepLeft = 0;

    std::vector<Element> m_elements;
    std::vector<Dog> m_dogs;
    std::vector<Sheep> m_sheep;
    std::vector<BarkWave> m_barkWaves;
    std::vector<GameEvent> m_events;
};

} // namespace game
