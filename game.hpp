#pragma once

#include "camera.hpp"
#include "entities.hpp"
#include "pch.h"
#include "renderer.hpp"

namespace Tmpl8 {

class Surface;
class Game {
public:
  ICamera *camera;

  void Init();
  void Shutdown();
  void Tick(float deltaTime);
  void MouseUp(
      int button) { /* implement if you want to detect mouse button presses */ }
  void MouseDown(
      int button) { /* implement if you want to detect mouse button presses */ }
  void MouseMove(float x, float y) { mouseRelativeX = x, mouseRelativeY = y; }
  void KeyUp(SDL_Scancode key) { released.set(key); }
  void KeyDown(SDL_Scancode key) { pressed.set(key); }
  [[nodiscard]] bool GetKey(SDL_Scancode key) const {
    return held.test(key);
  } // returns true if the key is currently held down
  [[nodiscard]] bool GetKeyPressed(SDL_Scancode key) const {
    return pressed.test(key);
  } // returns true if the key was pressed since the last Tick
  [[nodiscard]] bool GetKeyReleased(SDL_Scancode key) const {
    return released.test(key);
  } // returns true if the key was released since the last Tick
  void Screenshot();
  void SetPlayerCamera(shared_ptr<FollowCamera> &c) {
    playerCamera = c;
    camera = c.get();
  }
  void SetDebugCamera(unique_ptr<FreeCamera> &c) {
    debugCamera = std::move(c);
  };
  void SetRenderer(shared_ptr<Renderer> &r) { renderer = r; }
  void SetEntities(shared_ptr<Entities> &e) { entities = e; }
  void SetupKeys() {
    // Remember any newly pressed key while keeping the old ones
    held |= pressed;
    // Forget keys that were just released
    held ^= released;
    pressed.reset();
    released.reset();
  }
  void ToCheckpoint();

  // Use for pause/slowmo/fastmo
  float dtMultiplier = 1.f;

private:
  std::bitset<SDL_SCANCODE_COUNT> keys; // store key states here
  std::bitset<SDL_SCANCODE_COUNT>
      held; // store key states from the previous tick here
  std::bitset<SDL_SCANCODE_COUNT>
      pressed; // store key press events here (set to true on key down, reset to
               // false after processing in Tick)
  std::bitset<SDL_SCANCODE_COUNT>
      released; // store key release events here (set to true on key up, reset
                // to false after processing in Tick)

  float mouseRelativeX, mouseRelativeY;
  float prevMouseX = 0.0f, prevMouseY = 0.0f;
  shared_ptr<FollowCamera> playerCamera;
  unique_ptr<FreeCamera> debugCamera;
  shared_ptr<Renderer> renderer;
  shared_ptr<Entities> entities;
  bool usingDebugCamera = false;

  void toggleDebugCamera() {
    if (usingDebugCamera)
      camera = playerCamera.get();
    else {
      camera = debugCamera.get();
    }

    usingDebugCamera = !usingDebugCamera;
  }
};
}; // namespace Tmpl8
