// Tests for the software mixer (src/audio/mixer).
#include "audio/mixer.hpp"

#include <cmath>
#include <cstdio>
#include <vector>

using audio::Mixer;
using audio::SoundData;

namespace {

int gFailures = 0;

#define CHECK(cond)                                                              \
    do {                                                                         \
        if (!(cond)) {                                                           \
            std::printf("%s:%d: CHECK failed: %s\n", __FILE__, __LINE__, #cond); \
            ++gFailures;                                                         \
        }                                                                        \
    } while (0)

bool near(float a, float b) { return std::fabs(a - b) < 1e-5f; }

// constant stereo sound of `frames` frames
std::shared_ptr<SoundData> tone(size_t frames, float value)
{
    auto s = std::make_shared<SoundData>();
    s->samples.assign(frames * audio::kChannels, value);
    return s;
}

std::vector<float> mix(Mixer& m, size_t frames)
{
    std::vector<float> out(frames * audio::kChannels, 123.0f); // garbage, must be overwritten
    m.mix(out.data(), frames);
    return out;
}

void testSilenceAndSingleSound()
{
    Mixer m;
    auto out = mix(m, 4);
    for (float v : out)
        CHECK(v == 0.0f);

    m.play(tone(6, 0.25f));
    CHECK(m.activeVoices() == 1);
    out = mix(m, 4);
    for (float v : out)
        CHECK(near(v, 0.25f));
    // remaining 2 frames, then silence; the finished voice is removed
    out = mix(m, 4);
    CHECK(near(out[0], 0.25f) && near(out[3], 0.25f) && out[4] == 0.0f && out[7] == 0.0f);
    CHECK(m.activeVoices() == 0);
}

void testSumVolumeAndClamp()
{
    Mixer m;
    m.play(tone(10, 0.25f), 0.5f);
    m.play(tone(10, 0.5f));
    auto out = mix(m, 2);
    CHECK(near(out[0], 0.125f + 0.5f));

    m.masterVolume = 0.5f;
    out = mix(m, 2);
    CHECK(near(out[0], (0.125f + 0.5f) * 0.5f));

    // too loud: clamped instead of wrapping
    Mixer loud;
    loud.play(tone(4, 0.8f));
    loud.play(tone(4, 0.8f));
    out = mix(loud, 1);
    CHECK(out[0] == 1.0f);
    Mixer neg;
    neg.play(tone(4, -0.8f));
    neg.play(tone(4, -0.8f));
    CHECK(mix(neg, 1)[0] == -1.0f);
}

void testVoiceLimitAndMute()
{
    Mixer m(2);
    m.play(tone(100, 0.1f));
    m.play(tone(100, 0.2f));
    m.play(tone(100, 0.4f)); // replaces the oldest (0.1)
    CHECK(m.activeVoices() == 2);
    CHECK(near(mix(m, 1)[0], 0.6f));

    // muted: silent, but sounds keep running (no old sounds when unmuting)
    m.muted = true;
    CHECK(mix(m, 98)[0] == 0.0f);
    CHECK(m.activeVoices() == 2);
    mix(m, 1);
    CHECK(m.activeVoices() == 0);
    m.muted = false;
    CHECK(mix(m, 1)[0] == 0.0f);

    // empty or missing sounds are ignored
    m.play(nullptr);
    m.play(std::make_shared<SoundData>());
    CHECK(m.activeVoices() == 0);
    m.play(tone(4, 0.1f));
    m.stopAll();
    CHECK(m.activeVoices() == 0);
}

void testSharedOwnership()
{
    // a voice keeps its sound alive even if the caller drops it
    Mixer m;
    {
        auto s = tone(4, 0.3f);
        m.play(s);
    }
    CHECK(near(mix(m, 1)[0], 0.3f));
}

} // namespace

int main()
{
    testSilenceAndSingleSound();
    testSumVolumeAndClamp();
    testVoiceLimitAndMute();
    testSharedOwnership();

    if (gFailures == 0)
        std::printf("all mixer tests passed\n");
    return gFailures == 0 ? 0 : 1;
}
