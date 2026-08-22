#include "game.hpp"
#include "audio.hpp"

#include <windows.h>

namespace Tmpl8 {

int checkpointID = 0;
vector<vector<vec3>> checkpoints{
    {{9.826279640197754f, -0.024219999089837074f, 0.1309020072221756f}},
    {{53.835f, -.45012f, -3.5062f}}};

void Game::Init() {
  AudioMachine::Init();
}

void Game::Tick(float deltaTime) {
  camera->Update(deltaTime, entities->ProvideCameraFollow(),
                 entities->ProvideCameraForward());
  camera->HandleMouseMovement(mouseRelativeX, mouseRelativeY);

  // NOTE: Relative motion goes both ways, so when the mouse stops moving
  // the relative motion becomes negative and brings the mouse back to (0,0)
  mouseRelativeX = 0.0f;
  mouseRelativeY = 0.0f;

  if (GetKey(SDL_SCANCODE_RIGHT)) {
    entities->RegisterInputRight(deltaTime, camera->GetRight());
  }

  if (GetKey(SDL_SCANCODE_LEFT)) {
    entities->RegisterInputLeft(deltaTime, camera->GetRight());
  }

#ifdef _DEBUG
  if (GetKey(SDL_SCANCODE_UP)) {
    entities->RegisterInputForward(deltaTime, camera->GetForward());
  }

  if (GetKey(SDL_SCANCODE_DOWN)) {
    entities->RegisterInputBackward(deltaTime, camera->GetForward());
  }

  if (GetKeyReleased(SDL_SCANCODE_P) || GetKeyReleased(SDL_SCANCODE_0)) {
    dtMultiplier = dtMultiplier == 0.f ? 1.f : 0.f;
  }

  if (GetKeyReleased(SDL_SCANCODE_J)) {
    dtMultiplier = dtMultiplier <= 0.2f ? 0.1f : dtMultiplier - 0.1f;
  }

  if (GetKeyReleased(SDL_SCANCODE_K)) {
    dtMultiplier += 0.1f;
  }

  if (GetKeyReleased(SDL_SCANCODE_C)) {
    toggleDebugCamera();
  }

  if (GetKeyReleased(SDL_SCANCODE_M)) {
    debugHeap = true;
  }
#endif

  if (GetKey(SDL_SCANCODE_W)) {
    camera->HandleKeyboardUp(deltaTime);
  }

  if (GetKey(SDL_SCANCODE_S)) {
    camera->HandleKeyboardDown(deltaTime);
  }

  if (GetKey(SDL_SCANCODE_A)) {
    camera->HandleKeyboardLeft(deltaTime);
  }

  if (GetKey(SDL_SCANCODE_D)) {
    camera->HandleKeyboardRight(deltaTime);
  }

  if (GetKeyPressed(SDL_SCANCODE_RETURN)) {
    ToCheckpoint();
  }

  if (GetKeyPressed(SDL_SCANCODE_SPACE)) {
    entities->ToggleSplitMode();
  }

  if (entities->ProvideCameraFollow().x > 55.0f) {
    checkpointID = 1;
  }

  if (entities->ProvideCameraFollow().x > 180.0f) {
    renderer->ShowVictoryMessage();
    checkpointID = 0;
  }
}

void Game::Shutdown() {
  AudioMachine::CleanUp();
}

void Game::ToCheckpoint() {
  const auto &dynamicEntitiesPos = checkpoints[checkpointID];
  entities->ToCheckpoint(dynamicEntitiesPos);
  playerCamera->ToCheckpoint(entities->ProvideCameraFollow());
  renderer->ToCheckpoint();
}
} // namespace Tmpl8
