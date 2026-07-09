// Template, BUAS version https://www.buas.nl/games
// IGAD/BUAS(NHTV)/UU - Jacco Bikker - 2006-2020

// Note:
// this version of the template uses SDL2 for all frame buffer interaction
// see: https://www.libsdl.org

#ifdef _MSC_VER
#pragma warning(disable : 4530) // complaint about exception handler
#pragma warning(disable : 4311) // pointer truncation from HANDLE to long
#endif

#include "template.hpp"
#include "camera.hpp"
#include "game.hpp"
#include "physics.hpp"
#include "renderer.hpp"
#include <corecrt_math.h>
#include <cstdio>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <windows.h>

namespace Tmpl8 {

double timer::inv_freq = 1;

timer::timer() : start(get()) { init(); }

float timer::elapsed() const { return (float)((get() - start) * inv_freq); }

timer::value_type timer::get() {
  LARGE_INTEGER c;
  QueryPerformanceCounter(&c);
  return c.QuadPart;
}

double timer::to_time(const value_type vt) { return double(vt) * inv_freq; }

void timer::reset() { start = get(); }

void timer::init() {
  LARGE_INTEGER f;
  QueryPerformanceFrequency(&f);
  // NOTE: changed this from 1000. to 1.
  // Multiplying by 1000 here led to delta times above 1 sec for 240 frames a
  // second (anyway, logging would show like 242 when less then a second had
  // passed)
  // This also fixes massive performance issues with the physics update, which
  // was called way too many times a second (6000?) as a result
  inv_freq = 1. / double(f.QuadPart);
}

void NotifyUser(const char *s) {
  HWND hApp = FindWindow(nullptr, TemplateVersion);
  MessageBox(hApp, s, "ERROR", MB_OK);
  exit(0);
}

} // namespace Tmpl8

using namespace Tmpl8;
using namespace std;

int ACTWIDTH, ACTHEIGHT;

Game *game = nullptr;

#ifdef _MSC_VER
bool redirectIO() {
  CONSOLE_SCREEN_BUFFER_INFO coninfo;
  AllocConsole();
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
  coninfo.dwSize.Y = 500;
  SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);
  HANDLE h1 = GetStdHandle(STD_OUTPUT_HANDLE);
  int h2 = _open_osfhandle((intptr_t)h1, _O_TEXT);
  FILE *fp = _fdopen(h2, "w");
  *stdout = *fp;
  setvbuf(stdout, NULL, _IONBF, 0);
  h1 = GetStdHandle(STD_INPUT_HANDLE),
  h2 = _open_osfhandle((intptr_t)h1, _O_TEXT);
  fp = _fdopen(h2, "r"), *stdin = *fp;
  setvbuf(stdin, NULL, _IONBF, 0);
  h1 = GetStdHandle(STD_ERROR_HANDLE),
  h2 = _open_osfhandle((intptr_t)h1, _O_TEXT);
  fp = _fdopen(h2, "w"), *stderr = *fp;
  setvbuf(stderr, NULL, _IONBF, 0);
  ios::sync_with_stdio();
  FILE *stream;
  if ((stream = freopen("CON", "w", stdout)) == NULL)
    return false;
  if ((stream = freopen("CON", "w", stderr)) == NULL)
    return false;
  return true;
}
#endif

int main(int argc, char **argv) {
#ifdef _MSC_VER
  if (!redirectIO())
    return 1;
#endif
  int exitapp = 0;

  // NOTE: Must be initiated before entities, because initiates GL context
  shared_ptr<Renderer> renderer = std::make_shared<Renderer>();

  shared_ptr<Entities> entities = std::make_shared<Entities>();
  shared_ptr<FollowCamera> camera = std::make_shared<FollowCamera>(
      entities->ProvideCameraFollow(), Maths::vec3::up, 20.0f, 6.0f);

  renderer->SetCamera(camera);
  renderer->Init(entities);

  game = new Game();
  game->SetCamera(camera);
  game->SetRenderer(renderer);
  game->SetEntities(entities);

  ShowCursor(false);

  timer t;
  t.reset();

  float tPhysics = 0.0;
  float physicsTimeAccumulator = 0.0;

  game->Init();
  // NOTE: Only make game start once things are ready be rendered.
  t.reset();

  while (!exitapp) {
    // calculate frame time and pass it to game->Tick
    float elapsedTime = t.elapsed();
    t.reset();

    physicsTimeAccumulator += elapsedTime;

    // NOTE: make sure the physics update always gets the same delta time
    while (physicsTimeAccumulator >= Physics::physicsDeltaTime) {
      entities->Update(tPhysics, Physics::physicsDeltaTime);
      physicsTimeAccumulator -= Physics::physicsDeltaTime;
      tPhysics += Physics::physicsDeltaTime;
    }

    // NOTE: use to lerp between previous and next physics state
    // See: https://www.gafferongames.com/post/fix_your_timestep/
    const double alpha = physicsTimeAccumulator / Physics::physicsDeltaTime;

    game->Tick(elapsedTime);

    renderer->Draw3D(elapsedTime, entities);

    game->SetupKeys();

    // event loop
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_EVENT_QUIT:
        exitapp = 1;
        break;
      case SDL_EVENT_KEY_DOWN:
        if (event.key.key == SDLK_ESCAPE) {
          exitapp = 1;
          // find other keys here: http://sdl.beuc.net/sdl.wiki/SDLKey
        }
        game->KeyDown(event.key.scancode);
        break;
      case SDL_EVENT_KEY_UP:
        game->KeyUp(event.key.scancode);
        break;
      case SDL_EVENT_MOUSE_MOTION:
        game->MouseMove(event.motion.xrel, event.motion.yrel);
        break;
      case SDL_EVENT_MOUSE_BUTTON_UP:
        game->MouseUp(event.button.button);
        break;
      case SDL_EVENT_MOUSE_BUTTON_DOWN:
        game->MouseDown(event.button.button);
        break;
      default:
        break;
      }
    }
  }

  game->Shutdown();

  return 0;
}
