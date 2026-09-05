#ifndef _AUDIO_HPP
#define _AUDIO_HPP
#include "pch.h"

struct AudioMachine {
  AudioMachine() = delete;

  static constexpr int numTracks = 5;
  static constexpr const char *tracks[numTracks] = {
      "assets/Feint - for the fire instrumental.wav",
      "assets/Dualistic - Station Six.wav", "assets/Maduk_Levitate.wav",
      "assets/Monrroe_A_Place_To_Belong.wav",
      "assets/Rameses B - Once Upon A Time.wav"};

  static SDL_AudioSpec backtrackSpec;
  static u8 *backtrackBuffer;
  static u32 backtrackLength;
  static SDL_AudioStream *backtrackStream;

  static SDL_AudioSpec oneOffSpec;
  static u8 *oneOffBuffer;
  static u32 oneOffLength;
  static SDL_AudioStream *oneOffStream;

  static SDL_AudioSpec rollSpec;
  static u8 *rollBuffer;
  static u32 rollLength;
  static SDL_AudioStream *rollStream;

  static void Init() {
    // Load collision sound
    if (!SDL_LoadWAV("assets/marble-drop.wav", &oneOffSpec, &oneOffBuffer,
                     &oneOffLength)) {
      SDL_Log("Error: Failed to load audio: %s", SDL_GetError());
    }

    oneOffStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                                             &oneOffSpec, nullptr, nullptr);

    if (!oneOffStream) {
      SDL_Log("Error: Failed to open an audio device stream: %s",
              SDL_GetError());
    }

    // Load roll sound
    if (!SDL_LoadWAV("assets/rolling-cart.wav", &rollSpec, &rollBuffer,
                     &rollLength)) {
      SDL_Log("Error: Failed to load audio: %s", SDL_GetError());
    }

    rollStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                                           &rollSpec, nullptr, nullptr);

    if (!rollStream) {
      SDL_Log("Error: Failed to open an audio device stream: %s",
              SDL_GetError());
    }

    // Load background music
    std::random_device seed_gen;
    std::uint32_t seed = seed_gen();
    std::mt19937 engine(seed);
    std::uniform_int_distribution<int> dist(0, numTracks - 1);

    i32 trackIdx = dist(engine);

    if (!SDL_LoadWAV(tracks[trackIdx], &backtrackSpec, &backtrackBuffer,
                     &backtrackLength)) {
      SDL_Log("Error: Failed to load audio: %s", SDL_GetError());
    }

    backtrackStream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &backtrackSpec, nullptr, nullptr);

    if (!backtrackStream) {
      SDL_Log("Error: Failed to open an audio device stream: %s",
              SDL_GetError());
    }

    SDL_SetAudioStreamGain(backtrackStream, 0.15f);

    if (!SDL_PutAudioStreamData(backtrackStream, backtrackBuffer,
                                backtrackLength)) {
      SDL_Log("Error: Failed to put audio in the stream: %s", SDL_GetError());
    }

    SDL_ResumeAudioStreamDevice(backtrackStream);
  }

  static void PlayCollision(f32 collisionStrength) {
    if (collisionStrength < 1.f)
      return;

    SDL_ClearAudioStream(oneOffStream);

    SDL_SetAudioStreamGain(oneOffStream, log10f(collisionStrength));

    if (!SDL_PutAudioStreamData(oneOffStream, oneOffBuffer, oneOffLength)) {
      SDL_Log("Error: Failed to put audio in the stream: %s", SDL_GetError());
    }

    SDL_ResumeAudioStreamDevice(oneOffStream);
  }

  static void OnGround(f32 onryo) {
    auto queuedBytes = SDL_GetAudioStreamQueued(rollStream);

    if (queuedBytes < rollLength) {
      SDL_SetAudioStreamFrequencyRatio(rollStream, log10f(onryo + 1.f));

      if (!SDL_PutAudioStreamData(rollStream, rollBuffer, rollLength)) {
        SDL_Log("Error: Failed to put audio in the stream: %s", SDL_GetError());
      }

      SDL_ResumeAudioStreamDevice(rollStream);
    }
  }

  static void LeftGround() { SDL_ClearAudioStream(rollStream); }

  static void CleanUp() {
    SDL_free(backtrackBuffer);
    SDL_free(oneOffBuffer);
    SDL_free(rollBuffer);
    SDL_DestroyAudioStream(backtrackStream);
    SDL_DestroyAudioStream(oneOffStream);
    SDL_DestroyAudioStream(rollStream);
  }
};

#endif
