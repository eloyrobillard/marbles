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

  static void Init() {
    // Load collision sound
    if (!SDL_LoadWAV("assets/freesound_community-marble-drop-93150.wav",
                     &oneOffSpec, &oneOffBuffer, &oneOffLength)) {
      SDL_Log("Error: Failed to load audio: %s", SDL_GetError());
    }

    oneOffStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                                             &oneOffSpec, nullptr, nullptr);

    if (!oneOffStream) {
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
    SDL_PauseAudioStreamDevice(oneOffStream);

    SDL_SetAudioStreamGain(oneOffStream, collisionStrength);

    if (!SDL_PutAudioStreamData(oneOffStream, oneOffBuffer, oneOffLength)) {
      SDL_Log("Error: Failed to put audio in the stream: %s", SDL_GetError());
    }

    SDL_ResumeAudioStreamDevice(oneOffStream);
  }

  static void CleanUp() {
    SDL_free(backtrackBuffer);
    SDL_free(oneOffBuffer);
    SDL_DestroyAudioStream(backtrackStream);
    SDL_DestroyAudioStream(oneOffStream);
  }
};

#endif
