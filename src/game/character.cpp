#include "character.hpp"

#include "constants.hpp"

using core::Vec2;

namespace game {

Character::Character(CharacterType type, Vec2 mid, Vec2 size, Vec2 transformPoint, float distToColRect,
                     int numAnimationSteps)
    : m_type(type),
      m_mid(mid),
      m_prevMid(mid),
      m_size(size),
      m_transformPoint(transformPoint),
      m_distToColRect(distToColRect),
      m_numAnimationSteps(numAnimationSteps)
{
    const Vec2 colCenter = drawPosition() + m_transformPoint;
    m_colRect = {colCenter.x - distToColRect, colCenter.y - distToColRect, 2 * distToColRect, 2 * distToColRect};
}

float Character::rotationDeg() const
{
    return -m_angle - kCharacterRotationOffset;
}

void Character::changeSpeed(float speed)
{
    // reinit the speed vector if speed was zero before
    if (m_speed.isNull())
        m_speed = Vec2::fromPolarDeg(m_angle, 1.0f);
    m_speed = m_speed.withLength(speed);
}

void Character::changeAngle(float angleDeg)
{
    while (angleDeg < 0.0f)
        angleDeg += 360.0f;
    while (angleDeg > 360.0f)
        angleDeg -= 360.0f;
    m_angle = angleDeg;
    m_speed = m_speed.withAngleDeg(m_angle);
}

void Character::setSpeedAndAngle(float speed, float angleDeg)
{
    m_speed = Vec2(1.0f, 1.0f).withLength(speed);
    changeAngle(angleDeg);
}

void Character::updateAnimation()
{
    if (m_numAnimationSteps <= 1)
        return;
    if (m_animationForward) {
        if (m_animationStep == m_numAnimationSteps - 1) {
            --m_animationStep;
            m_animationForward = false;
        } else {
            ++m_animationStep;
        }
    } else {
        if (m_animationStep == 0) {
            ++m_animationStep;
            m_animationForward = true;
        } else {
            --m_animationStep;
        }
    }
}

void Character::move(Vec2 d)
{
    if (d.isNull())
        return;
    m_mid += d;
    m_colRect.translate(d);
}

Dog::Dog(Vec2 mid)
    : Character(CharacterType::Dog, mid, kDogImageSize,
                // rotation centre and collision area are near the head end
                {kDogImageSize.x / 2.0f, kDogImageSize.y - 26.0f}, (kDogImageSize.x + kDogImageSize.y) / 6.0f,
                kNumDogAnimationSteps),
      m_barkPower(kMaxBarkPower)
{
    setSpeedAndAngle(0.0f, 0.0f);
}

Sheep::Sheep(Vec2 mid, float speed, float angleDeg, long baaTick)
    : Character(CharacterType::Sheep, mid, kSheepImageSize, kSheepImageSize * 0.5f,
                (kSheepImageSize.x + kSheepImageSize.y) / 4.0f, kNumSheepAnimationSteps),
      m_baaTick(baaTick)
{
    setSpeedAndAngle(speed, angleDeg);
}

} // namespace game
