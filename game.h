#pragma once

#include "pch.h"

namespace Tmpl8 {

class Surface;
class Game {
public:
  void SetTarget(Surface *surface) { screen = surface; }
  void Init();
  void Shutdown();
  void Tick(float deltaTime);
  void PhysicsTick(double t, double dt);
  void MouseUp(
      int button) { /* implement if you want to detect mouse button presses */ }
  void MouseDown(
      int button) { /* implement if you want to detect mouse button presses */ }
  void MouseMove(int x, int y) { mousex = x, mousey = y; }
  void KeyUp(SDL_Scancode key) { keys.reset(key); }
  void KeyDown(SDL_Scancode key) { keys.set(key); }
  bool GetKey(SDL_Scancode key) const {
    return held.test(key);
  } // returns true if the key is currently held down
  bool GetKeyPressed(SDL_Scancode key) const {
    return pressed.test(key);
  } // returns true if the key was pressed since the last Tick
  bool GetKeyReleased(SDL_Scancode key) const {
    return released.test(key);
  } // returns true if the key was released since the last Tick
  void Screenshot();

  double physicsTicksPerSecond = 60.0;
  double physicsDeltaTime = 1.0 / physicsTicksPerSecond;

private:
  Surface *screen;
  std::bitset<SDL_NUM_SCANCODES> keys; // store key states here
  std::bitset<SDL_NUM_SCANCODES>
      held; // store key states from the previous tick here
  std::bitset<SDL_NUM_SCANCODES>
      pressed; // store key press events here (set to true on key down, reset to
               // false after processing in Tick)
  std::bitset<SDL_NUM_SCANCODES>
      released; // store key release events here (set to true on key up, reset
                // to false after processing in Tick)
  int mousex, mousey;
};

}; // namespace Tmpl8
