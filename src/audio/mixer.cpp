#include "mixer.hpp"

#include <algorithm>

namespace audio {

void Mixer::play(std::shared_ptr<const SoundData> sound, float volume)
{
    if (!sound || sound->frames() == 0 || m_maxVoices == 0)
        return;
    if (m_voices.size() >= m_maxVoices)
        m_voices.erase(m_voices.begin());
    m_voices.push_back({std::move(sound), 0, volume});
}

void Mixer::mix(float* out, size_t frames)
{
    std::fill(out, out + frames * kChannels, 0.0f);
    const float master = muted ? 0.0f : masterVolume;

    for (Voice& v : m_voices) {
        const size_t n = std::min(frames, v.sound->frames() - v.frame);
        const float gain = v.volume * master;
        const float* src = v.sound->samples.data() + v.frame * kChannels;
        for (size_t i = 0; i < n * kChannels; ++i)
            out[i] += src[i] * gain;
        v.frame += n; // muted sounds keep running, so unmuting doesn't replay old sounds
    }
    std::erase_if(m_voices, [](const Voice& v) { return v.frame >= v.sound->frames(); });

    for (size_t i = 0; i < frames * kChannels; ++i)
        out[i] = std::clamp(out[i], -1.0f, 1.0f);
}

} // namespace audio
