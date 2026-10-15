#include "audio.hpp"

#include "core/strings.hpp"
#include "platform/assets.hpp"

#include <SDL3/SDL.h>

namespace audio {

namespace {

constexpr SDL_AudioSpec kSpec{SDL_AUDIO_F32, kChannels, kSampleRate};

// loads a WAV in any PCM format and converts it to the mixer's format
std::shared_ptr<SoundData> loadWav(const std::string& path)
{
    SDL_AudioSpec spec;
    Uint8* data = nullptr;
    Uint32 length = 0;
    if (!SDL_LoadWAV(path.c_str(), &spec, &data, &length)) {
        SDL_Log("sound %s: %s", path.c_str(), SDL_GetError());
        return nullptr;
    }
    Uint8* converted = nullptr;
    int convertedLength = 0;
    const bool ok = SDL_ConvertAudioSamples(&spec, data, static_cast<int>(length), &kSpec, &converted, &convertedLength);
    SDL_free(data);
    if (!ok) {
        SDL_Log("sound %s: conversion failed: %s", path.c_str(), SDL_GetError());
        return nullptr;
    }
    auto sound = std::make_shared<SoundData>();
    const auto* samples = reinterpret_cast<const float*>(converted);
    sound->samples.assign(samples, samples + static_cast<size_t>(convertedLength) / sizeof(float));
    SDL_free(converted);
    return sound;
}

} // namespace

std::unique_ptr<Audio> Audio::open()
{
    std::unique_ptr<Audio> audio(new Audio);
    audio->m_stream =
        SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &kSpec, &Audio::callback, audio.get());
    if (!audio->m_stream) {
        SDL_Log("no audio: %s", SDL_GetError());
        return nullptr;
    }
    audio->m_mixer.masterVolume = 0.8f;
    SDL_ResumeAudioStreamDevice(audio->m_stream);
    return audio;
}

Audio::~Audio()
{
    if (m_stream)
        SDL_DestroyAudioStream(m_stream); // stops the callback before the mixer goes away
}

void Audio::callback(void* userdata, SDL_AudioStream* stream, int additional, int /*total*/)
{
    // runs on the audio thread with the stream locked
    auto* self = static_cast<Audio*>(userdata);
    const size_t frames = static_cast<size_t>(additional) / (sizeof(float) * kChannels);
    if (frames == 0)
        return;
    self->m_buffer.resize(frames * kChannels);
    self->m_mixer.mix(self->m_buffer.data(), frames);
    SDL_PutAudioStreamData(stream, self->m_buffer.data(), static_cast<int>(frames * kChannels * sizeof(float)));
}

std::shared_ptr<const SoundData> Audio::sound(const std::string& name)
{
    auto it = m_sounds.find(name);
    if (it == m_sounds.end())
        it = m_sounds.emplace(name, loadWav(platform::assetPath("sfx/" + name + ".wav"))).first;
    return it->second;
}

void Audio::play(std::string_view name, int variants, float volume)
{
    std::string file(name);
    if (variants > 1)
        file += std::to_string(std::uniform_int_distribution<int>(1, variants)(m_random));
    auto s = sound(file); // loaded outside the stream lock
    if (!s)
        return;
    ++m_playCount;
    SDL_LockAudioStream(m_stream);
    m_mixer.play(std::move(s), volume);
    SDL_UnlockAudioStream(m_stream);
}

void Audio::preload(std::string_view name, int variants)
{
    if (variants <= 1) {
        sound(std::string(name));
        return;
    }
    for (int i = 1; i <= variants; ++i)
        sound(std::string(name) + std::to_string(i));
}

void Audio::setMuted(bool muted)
{
    m_muted = muted;
    SDL_LockAudioStream(m_stream);
    m_mixer.muted = muted;
    SDL_UnlockAudioStream(m_stream);
}

void Audio::pauseDevice()
{
    SDL_PauseAudioStreamDevice(m_stream);
}

void Audio::resumeDevice()
{
    SDL_ResumeAudioStreamDevice(m_stream);
}

size_t Audio::soundsLoaded() const
{
    size_t n = 0;
    for (const auto& [name, s] : m_sounds)
        n += s ? 1 : 0;
    return n;
}

} // namespace audio
