#pragma once

#include "mixer.hpp"

#include <map>
#include <memory>
#include <random>
#include <string>
#include <string_view>

struct SDL_AudioStream;

// Sound output: plays the WAVs in assets/sfx through SDL's audio device and the Mixer.
namespace audio {

class Audio {
public:
    // nullptr if no audio device could be opened (the game then runs silently)
    static std::unique_ptr<Audio> open();
    ~Audio();
    Audio(const Audio&) = delete;
    Audio& operator=(const Audio&) = delete;

    // Plays sfx/<name>.wav. With variants > 1, one of <name>1 .. <name><variants> at random
    // (e.g. play("bark", 4)). Unknown sounds are logged once and ignored.
    void play(std::string_view name, int variants = 1, float volume = 1.0f);
    // loads sounds up front, so playing them later doesn't stall a frame
    void preload(std::string_view name, int variants = 1);

    void setMuted(bool muted);
    bool muted() const { return m_muted; }
    // stop output while the app is in the background
    void pauseDevice();
    void resumeDevice();

    size_t soundsLoaded() const;
    size_t playCount() const { return m_playCount; }

private:
    Audio() = default;
    std::shared_ptr<const SoundData> sound(const std::string& name);
    static void callback(void* userdata, SDL_AudioStream* stream, int additional, int total);

    SDL_AudioStream* m_stream = nullptr;
    Mixer m_mixer;
    std::vector<float> m_buffer;
    std::map<std::string, std::shared_ptr<const SoundData>> m_sounds; // nullptr: failed to load
    std::minstd_rand m_random{std::random_device{}()};
    bool m_muted = false;
    size_t m_playCount = 0;
};

} // namespace audio
