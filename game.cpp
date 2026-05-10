#include "game.h"

#include <windows.h>

namespace Tmpl8 {

int checkpointID = 0;
vector<vector<vec3>> checkpoints{
    {{9.826279640197754, -0.024219999089837074, 0.1309020072221756}},
    {{53.835, -.45012, -3.5062}}};

SDL_AudioSpec audioSpec;
unsigned char *audioBuf;
uint audioLength;
SDL_AudioStream *audioStream;

void Game::Init() {
  // Launch background music
  if (!SDL_LoadWAV("assets/Dualistic - Station Six.wav", &audioSpec, &audioBuf,
                   &audioLength)) {
    SDL_Log("Error: Failed to load audio: %s", SDL_GetError());
  }

  audioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                                          &audioSpec, nullptr, nullptr);

  if (!audioStream) {
    SDL_Log("Error: Failed to open an audio device stream: %s", SDL_GetError());
  }

  SDL_SetAudioStreamGain(audioStream, 0.5f);

  if (!SDL_PutAudioStreamData(audioStream, audioBuf, audioLength)) {
    SDL_Log("Error: Failed to put audio in the stream: %s", SDL_GetError());
  }

  SDL_ResumeAudioStreamDevice(audioStream);
}

void Game::Tick(float deltaTime) {
  if (GetKey(SDL_SCANCODE_RIGHT)) {
    entities->RegisterInputRight(deltaTime);
  }

  if (GetKey(SDL_SCANCODE_LEFT)) {
    entities->RegisterInputLeft(deltaTime);
  }

  if (GetKeyPressed(SDL_SCANCODE_SPACE)) {
    ToCheckpoint();
  }

  if (entities->GetDynamicEntities()[0].GetPositionAsRef().x > 55.0f) {
    checkpointID = 1;
  }

  if (entities->GetDynamicEntities()[0].GetPositionAsRef().x > 180.0f) {
    renderer->ShowVictoryMessage();
    checkpointID = 0;
  }
}

void Game::Shutdown() {
  SDL_free(audioBuf);
  SDL_DestroyAudioStream(audioStream);
}

void Game::ToCheckpoint() {
  const auto &dynamicEntitiesPos = checkpoints[checkpointID];
  entities->ToCheckpoint(dynamicEntitiesPos);
  camera->ToCheckpoint(entities->ProvideCameraFollow());
  renderer->ToCheckpoint();
}
} // namespace Tmpl8
