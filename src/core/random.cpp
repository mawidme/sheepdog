#include "random.hpp"

#include <random>

namespace core {

namespace {
std::mt19937& engine()
{
    static std::mt19937 gen{std::random_device{}()};
    return gen;
}
} // namespace

void seedRandom(std::uint32_t seed)
{
    engine().seed(seed);
}

int randomInt(int minimum, int maximum)
{
    if (minimum >= maximum)
        return minimum;
    return std::uniform_int_distribution<int>(minimum, maximum)(engine());
}

float randomFloat(float minimum, float maximum)
{
    if (minimum >= maximum)
        return minimum;
    return std::uniform_real_distribution<float>(minimum, maximum)(engine());
}

} // namespace core
