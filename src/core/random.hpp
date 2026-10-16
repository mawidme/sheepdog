#pragma once

#include <cstdint>

namespace core {

// Game-wide random numbers. Replaces getRandom()/getRandomF() on top of rand().
void seedRandom(std::uint32_t seed);

// uniform in [minimum, maximum], both inclusive; returns minimum if the range is invalid
int randomInt(int minimum, int maximum);

// uniform in [minimum, maximum); returns minimum if the range is invalid
float randomFloat(float minimum, float maximum);

} // namespace core
