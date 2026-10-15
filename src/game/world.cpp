#include "world.hpp"

#include "core/random.hpp"

#include <algorithm>
#include <cmath>

using core::Rect;
using core::Vec2;

namespace game {

namespace {

constexpr int kNumWalkDirections = 4;

long nextBaaTick(long now)
{
    // baa in 10 + random[0, 10] seconds
    return now + kTicksPerSecond * (10 + core::randomInt(0, 10));
}

} // namespace

World::World(const LevelData& level)
    : m_size(static_cast<float>(level.width), static_cast<float>(level.height))
{
    // same order as the Qt version saved them (relevant: first element hit wins in collision checks)
    for (const FenceData& f : level.fences)
        m_elements.push_back(makeFence(f.from, f.to));
    for (const PropData& b : level.bushes)
        m_elements.push_back(makeBush(b.pos, b.style));
    for (const PropData& t : level.trees)
        m_elements.push_back(makeTree(t.pos, t.style));
    for (const WaterData& w : level.water)
        if (auto e = makeWater(w.from, w.to, w.style))
            m_elements.push_back(*e);
    for (const PropData& e : level.exits)
        m_elements.push_back(makeExit(e.pos, e.style));

    for (Vec2 p : level.dogs) {
        m_dogs.emplace_back(p);
        registerCharacter(m_dogs.back());
    }
    for (Vec2 p : level.sheep) {
        m_sheep.emplace_back(p, tuning.normalSheepSpeed, core::randomFloat(1.0f, 360.0f), nextBaaTick(m_tick));
        registerCharacter(m_sheep.back());
    }
    m_sheepLeft = static_cast<int>(m_sheep.size());
}

void World::registerCharacter(const Character& c)
{
    // collision checks must not step over a character's collision area
    if (m_colCheckStep > 2 * c.collisionRadius())
        m_colCheckStep = 2 * c.collisionRadius() - 1.0f;
}

std::vector<GameEvent> World::takeEvents()
{
    std::vector<GameEvent> events;
    events.swap(m_events);
    return events;
}

void World::tick()
{
    beginTick();
    advanceCharacters();
}

void World::beginTick()
{
    ++m_tick;
    for (Dog& d : m_dogs)
        d.beginTick();
    for (Sheep& s : m_sheep)
        s.beginTick();

    updateBarkWaves();
}

void World::advanceCharacters()
{
    // Qt: QGraphicsScene::advance() phase 0 (animations), then phase 1 (movement).
    // Dogs move before sheep, so sheep react to the dog's new position.
    for (Dog& d : m_dogs)
        d.updateAnimation();
    for (Sheep& s : m_sheep)
        s.updateAnimation();
    for (Dog& d : m_dogs)
        advanceDog(d);
    for (Sheep& s : m_sheep)
        advanceSheep(s);
}

void World::updateBarkWaves()
{
    for (BarkWave& w : m_barkWaves)
        w.radius += w.growthPerTick;
    std::erase_if(m_barkWaves, [](const BarkWave& w) { return w.radius > w.maxRadius; });
}

void World::advanceDog(Dog& dog)
{
    // Only future collisions are checked, so a standing dog has nothing to do.
    if (dog.speed() > kZeroSpeedThreshold) {
        // only walk until the target is reached (mouse / touch control)
        if (dog.target && distance(*dog.target, dog.position()) <= tuning.dogSpeed) {
            dog.changeSpeed(0.0f);
            dog.target.reset();
        } else {
            checkCollisions(dog);
            dog.move(dog.velocity());
        }
    }

    if (dog.m_barkPower < kMaxBarkPower) {
        if (dog.m_barkPower + tuning.barkPowerReload >= kMaxBarkPower)
            m_events.push_back(GameEvent::BarkPowerFull);
        dog.m_barkPower = std::min(dog.m_barkPower + tuning.barkPowerReload, kMaxBarkPower);
    }
}

void World::advanceSheep(Sheep& sheep)
{
    if (sheep.rescued || sheep.sleeping)
        return;

    if (m_tick == sheep.m_baaTick) {
        m_events.push_back(GameEvent::SheepBaa);
        sheep.m_baaTick = nextBaaTick(m_tick);
    }

    if (sheep.blocked) {
        // look for a way out every few ticks
        if (m_tick % kBlockedSheepRetryTicks != 0)
            return;
        sheep.blocked = false;
        sheep.changeSpeed(tuning.normalSheepSpeed);
    }

    // slow down again after being barked at
    if (sheep.speed() > tuning.normalSheepSpeed)
        sheep.changeSpeed(std::max(tuning.normalSheepSpeed, sheep.speed() - tuning.sheepSlowDown));

    // on collision, the new position and speed are set in checkCollisions()
    if (checkCollisions(sheep)) {
        if (sheep.speed() == 0.0f)
            sheep.blocked = true;
    } else {
        sheep.move(sheep.velocity());
    }
}

bool World::collidesAt(const Character& c, const Rect& r) const
{
    for (const Element& e : m_elements) {
        // dogs walk through exits
        if (!e.collision || (c.type() == CharacterType::Dog && e.type == ElementType::Exit))
            continue;
        if (e.collision->intersects(r))
            return true;
    }
    if (c.type() == CharacterType::Dog) {
        for (const Sheep& s : m_sheep)
            if (!s.rescued && s.collisionRect().intersects(r))
                return true;
    } else {
        for (const Dog& d : m_dogs)
            if (d.collisionRect().intersects(r))
                return true;
    }
    return false;
}

bool World::checkCollisions(Character& c)
{
    const Vec2 speedVector = c.velocity();
    const float speed = speedVector.length();
    const Rect& colRect = c.collisionRect();

    // Sheep are not moving as fast as the collision check step most of the time.
    bool oneStepOnly = false;
    float startWalkDist = m_colCheckStep;
    if (startWalkDist > speed) {
        startWalkDist = speed;
        oneStepOnly = true;
    }

    // last ok position before a collision; assume the whole distance may be walked
    Rect lastOkColRect = colRect.translated(speedVector);
    float lastOkWalkDist = speed;
    const Rect* nearestOtherColRect = nullptr;

    // Walk the distance in steps of m_colCheckStep and check each step against everything.
    bool breakOnNextDist = false;
    float prevWalkDist = 0.0f;
    float curWalkDist = startWalkDist;
    while (true) {
        const Rect checkRect = colRect.translated(speedVector.withLength(curWalkDist));

        // Collision rects to check, in the order of the Qt version: elements, then characters.
        const Rect* hit = nullptr;
        const Element* hitElement = nullptr;
        for (const Element& e : m_elements) {
            if (!e.collision || (c.type() == CharacterType::Dog && e.type == ElementType::Exit))
                continue;
            if (e.collision->intersects(checkRect)) {
                hit = &*e.collision;
                hitElement = &e;
                break;
            }
        }
        if (!hit) {
            if (c.type() == CharacterType::Dog) {
                for (const Sheep& s : m_sheep) {
                    // rescued sheep have left the level
                    if (!s.rescued && s.collisionRect().intersects(checkRect)) {
                        hit = &s.collisionRect();
                        break;
                    }
                }
            } else {
                for (const Dog& d : m_dogs) {
                    if (d.collisionRect().intersects(checkRect)) {
                        hit = &d.collisionRect();
                        break;
                    }
                }
            }
        }

        if (hit) {
            if (oneStepOnly || prevWalkDist < lastOkWalkDist) {
                nearestOtherColRect = hit;
                lastOkWalkDist = prevWalkDist;
                lastOkColRect = colRect.translated(speedVector.withLength(lastOkWalkDist));
            }
            if (hitElement && hitElement->type == ElementType::Exit && c.type() == CharacterType::Sheep) {
                rescue(static_cast<Sheep&>(c));
                return true;
            }
            break;
        }

        prevWalkDist = curWalkDist;
        if (breakOnNextDist || curWalkDist >= speed)
            break;
        // the speed is probably not a multiple of the step: make sure it is checked itself
        if (curWalkDist + m_colCheckStep >= speed) {
            breakOnNextDist = true;
            curWalkDist = speed;
        } else {
            curWalkDist += m_colCheckStep;
        }
    }

    if (!nearestOtherColRect)
        return false;

    Vec2 lastOkWalkVector = speedVector.withLength(lastOkWalkDist);

    // find the exact possible walking distance before the collision
    if (speed > kSpeedResolution) {
        float prevDist = 0.0f;
        for (float dist = lastOkWalkDist; dist <= lastOkWalkDist + m_colCheckStep; dist += kSpeedResolution) {
            lastOkWalkVector = speedVector.withLength(dist);
            if (nearestOtherColRect->intersects(colRect.translated(lastOkWalkVector))) {
                lastOkWalkDist = prevDist;
                lastOkWalkVector = speedVector.withLength(lastOkWalkDist);
                lastOkColRect = colRect.translated(lastOkWalkVector);
                break;
            }
            prevDist = dist;
        }
    }

    if (c.type() == CharacterType::Dog) {
        c.changeSpeed(0.0f);
        c.move(lastOkWalkVector);
        return true;
    }

    // Sheep: back to normal speed, then look for a way out in steps of 90 degrees and
    // walk that way, plus a random angle.
    c.changeSpeed(tuning.normalSheepSpeed);
    const float remainingWalkDist = c.speed();

    int stepAngle = static_cast<int>(std::floor(c.angle() / 90.0f)) * 90;
    Vec2 walkVectors[kNumWalkDirections];
    bool colDir[kNumWalkDirections];
    int numColDirs = 0;
    for (int i = 0; i < kNumWalkDirections; ++i) {
        walkVectors[i] = Vec2(remainingWalkDist, 0.0f).withAngleDeg(static_cast<float>(stepAngle));
        stepAngle = (stepAngle + 90) % 360;
        colDir[i] = collidesAt(c, lastOkColRect.translated(walkVectors[i]));
        if (colDir[i])
            ++numColDirs;
    }

    // No axis direction blocked: the obstacle was hit at a corner, diagonally. It lies in the
    // movement quadrant spanned by directions 0 and 1 (walkVectors start at floor(angle / 90) * 90),
    // so those are blocked. (The Qt version stopped the sheep as if blocked on all sides.)
    if (numColDirs == 0) {
        colDir[0] = true;
        colDir[1] = c.angle() != static_cast<float>(static_cast<int>(std::floor(c.angle() / 90.0f)) * 90);
        numColDirs = colDir[1] ? 2 : 1;
    }

    int firstNoColDir = 0;
    while (firstNoColDir < kNumWalkDirections && colDir[firstNoColDir])
        ++firstNoColDir;

    float newAngle = 0.0f;
    if (numColDirs == 1) {
        // turn away from the blocked neighbour direction
        if (colDir[(firstNoColDir + 1) % 4])
            newAngle = walkVectors[firstNoColDir].angleDeg() - static_cast<float>(core::randomInt(0, 45));
        else
            newAngle = walkVectors[firstNoColDir].angleDeg() + static_cast<float>(core::randomInt(0, 45));
    } else if (numColDirs == 2) {
        // Free directions adjacent: turn from the first free one towards the other free one.
        // (The Qt version also turned "+" when the other free one was at -90 degrees.)
        if (!colDir[(firstNoColDir + 1) % 4]) {
            newAngle = walkVectors[firstNoColDir].angleDeg() + static_cast<float>(core::randomInt(0, 45));
        } else if (!colDir[(firstNoColDir + 3) % 4]) {
            newAngle = walkVectors[firstNoColDir].angleDeg() - static_cast<float>(core::randomInt(0, 45));
        } else {
            // free directions are opposite: pick one
            newAngle = core::randomInt(0, 1) == 0 ? walkVectors[firstNoColDir].angleDeg()
                                                  : walkVectors[firstNoColDir + 2].angleDeg();
        }
    } else if (numColDirs == 3) {
        // only one way out
        newAngle = walkVectors[firstNoColDir].angleDeg();
    } else {
        // blocked on all sides
        c.changeSpeed(0.0f);
        return true;
    }

    const Vec2 escapeVector = Vec2::fromPolarDeg(newAngle, remainingWalkDist);
    c.changeAngle(newAngle);
    c.move(lastOkWalkVector + escapeVector);
    return true;
}

void World::rescue(Sheep& sheep)
{
    sheep.rescued = true;
    --m_sheepLeft;
    m_events.push_back(GameEvent::SheepRescued);
}

void World::walkTo(size_t dogIndex, Vec2 target)
{
    if (dogIndex >= m_dogs.size())
        return;
    Dog& dog = m_dogs[dogIndex];
    dog.target = target;
    dog.setSpeedAndAngle(tuning.dogSpeed, (target - dog.position()).angleDeg());
}

bool World::bark(size_t dogIndex)
{
    if (dogIndex >= m_dogs.size() || m_sheep.empty())
        return false;
    Dog& dog = m_dogs[dogIndex];

    if (dog.m_barkPower < kNeededBarkPower) {
        m_events.push_back(GameEvent::NoBark);
        return false;
    }
    m_events.push_back(GameEvent::Bark);
    dog.m_barkPower -= kNeededBarkPower;

    m_barkWaves.push_back({dog.position(), dog.collisionRect().w / 2.0f, tuning.maxBarkDist - kBarkWaveWidth / 2.0f, 0.0f});
    BarkWave& wave = m_barkWaves.back();
    wave.growthPerTick = (wave.maxRadius - wave.radius) / (kBarkWaveDuration * static_cast<float>(kTicksPerSecond));

    for (Sheep& s : m_sheep) {
        // sleeping sheep (sleep bonus) don't wake up
        if (s.sleeping || s.rescued)
            continue;
        const float dist = distance(s.position(), dog.position());
        if (dist > tuning.maxBarkDist)
            continue;
        // Adds speed, more the closer the dog is: as much as a normally walking sheep got in the
        // Qt version, which multiplied the current speed (so standing sheep ignored barks).
        const float boost = tuning.barkIntensity / std::max(dist, 1.0f) * kNormalSheepSpeed;
        s.changeSpeed(std::min(s.speed() + boost, tuning.maxSheepSpeed));
        s.changeAngle((s.position() - dog.position()).angleDeg());
    }

    if (dog.m_barkPower < 2 * kNeededBarkPower && dog.m_barkPower >= kNeededBarkPower)
        m_events.push_back(GameEvent::BarkPowerCritical);
    return true;
}

} // namespace game
