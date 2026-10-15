#pragma once

#include <cstddef>
#include <memory>
#include <vector>

// Software mixer for short sound effects; no SDL, so it can be unit tested.
namespace audio {

inline constexpr int kChannels = 2;
inline constexpr int kSampleRate = 44100;

// decoded sound: interleaved stereo float samples at kSampleRate
struct SoundData {
    std::vector<float> samples;
    size_t frames() const { return samples.size() / kChannels; }
};

class Mixer {
public:
    explicit Mixer(size_t maxVoices = 16) : m_maxVoices(maxVoices) {}

    // starts a sound; when all voices are busy, the one playing longest is replaced
    void play(std::shared_ptr<const SoundData> sound, float volume = 1.0f);
    void stopAll() { m_voices.clear(); }

    // writes `frames` stereo frames to out (overwriting it), clamped to [-1, 1]
    void mix(float* out, size_t frames);

    size_t activeVoices() const { return m_voices.size(); }
    float masterVolume = 1.0f;
    bool muted = false;

private:
    struct Voice {
        std::shared_ptr<const SoundData> sound;
        size_t frame = 0;
        float volume = 1.0f;
    };
    size_t m_maxVoices;
    std::vector<Voice> m_voices; // oldest first
};

} // namespace audio
