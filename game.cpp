#include "game.hpp"

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

  SDL_SetAudioStreamGain(audioStream, 0.15f);

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

#ifdef _DEBUG
  if (GetKey(SDL_SCANCODE_UP)) {
    entities->RegisterInputForward(deltaTime);
  }

  if (GetKey(SDL_SCANCODE_DOWN)) {
    entities->RegisterInputBackward(deltaTime);
  }
#endif

  if (GetKeyPressed(SDL_SCANCODE_RETURN)) {
    ToCheckpoint();
  }

  if (GetKeyPressed(SDL_SCANCODE_SPACE)) {
    entities->ToggleSplitMode();
  }

  camera->Update(deltaTime, entities->ProvideCameraFollow(),
                 entities->ProvideCameraForward());
  camera->SetMouseMovement(mouseRelativeX, mouseRelativeY);

  // NOTE: Relative motion goes both ways, so when the mouse stops moving
  // the relative motion becomes negative and brings the mouse back to (0,0)
  mouseRelativeX = 0.0f;
  mouseRelativeY = 0.0f;

  if (entities->ProvideCameraFollow().x > 55.0f) {
    checkpointID = 1;
  }

  if (entities->ProvideCameraFollow().x > 180.0f) {
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
