#pragma once

#include "core/rect.hpp"
#include "core/vec2.hpp"

#include <optional>

// Dog and sheep state. Replaces Character/Dog/Sheep of the Qt version; the movement and
// collision logic that needs the other objects lives in World.
namespace game {

enum class CharacterType { Dog, Sheep };

class Character {
public:
    CharacterType type() const { return m_type; }

    // centre in level coordinates
    core::Vec2 position() const { return m_mid; }
    // position at the start of the current tick, for interpolated drawing
    core::Vec2 previousPosition() const { return m_prevMid; }

    core::Vec2 size() const { return m_size; }
    // top-left corner of the (unrotated) image, as QGraphicsItem::pos()
    core::Vec2 drawPosition() const { return m_mid - m_size * 0.5f; }
    // rotation centre relative to the image's top-left corner
    core::Vec2 rotationPivot() const { return m_transformPoint; }
    // clockwise degrees for drawing (images face down)
    float rotationDeg() const;

    const core::Rect& collisionRect() const { return m_colRect; }
    // half the side length of the (square) collision rect
    float collisionRadius() const { return m_distToColRect; }

    // Zero degrees is at 3 o'clock, positive is counter-clockwise (QLineF convention).
    float angle() const { return m_angle; }
    float speed() const { return m_speed.length(); }
    core::Vec2 velocity() const { return m_speed; }

    // Same semantics as the Qt version, including their quirks:
    // changeSpeed() on a zero vector restarts in direction angle(); changeAngle() keeps a zero vector zero.
    void changeSpeed(float speed);
    void changeAngle(float angleDeg);
    void setSpeedAndAngle(float speed, float angleDeg);

    // animation step for the image (0-based), ping-pong through all steps
    int animationStep() const { return m_animationStep; }
    void updateAnimation();

    // moves by d without collision checks
    void move(core::Vec2 d);

    void beginTick() { m_prevMid = m_mid; }

protected:
    Character(CharacterType type, core::Vec2 mid, core::Vec2 size, core::Vec2 transformPoint, float distToColRect,
              int numAnimationSteps);

    // for the zeroSpeedX/Y quirk of the dog: only clears one component, the angle stays
    core::Vec2 m_speed;

private:
    CharacterType m_type;
    core::Vec2 m_mid;
    core::Vec2 m_prevMid;
    core::Vec2 m_size;
    core::Vec2 m_transformPoint; // also the collision rect centre, relative to the image
    float m_distToColRect;
    core::Rect m_colRect;
    float m_angle = 0.0f;

    int m_numAnimationSteps;
    int m_animationStep = 0;
    bool m_animationForward = true;
};

class Dog : public Character {
public:
    explicit Dog(core::Vec2 mid);

    float barkPower() const { return m_barkPower; }

    // walk towards a point (touch / mouse control); stops when it is reached
    std::optional<core::Vec2> target;

    // keyboard control: releasing a key only clears that component of the speed
    void zeroSpeedX() { m_speed.x = 0.0f; }
    void zeroSpeedY() { m_speed.y = 0.0f; }

private:
    friend class World;
    float m_barkPower;
};

class Sheep : public Character {
public:
    Sheep(core::Vec2 mid, float speed, float angleDeg, long baaTick);

    bool rescued = false;  // reached an exit, no longer part of the level
    bool blocked = false;  // collisions on all sides, retries regularly
    bool sleeping = false; // sleep bonus mode

private:
    friend class World;
    long m_baaTick;
};

} // namespace game
