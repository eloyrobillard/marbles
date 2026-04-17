// Template, BUAS version https://www.buas.nl/games
// IGAD/BUAS(NHTV)/UU - Jacco Bikker - 2006-2020

// Note:
// this version of the template uses SDL2 for all frame buffer interaction
// see: https://www.libsdl.org

#ifdef _MSC_VER
#pragma warning(disable : 4530) // complaint about exception handler
#pragma warning(disable : 4311) // pointer truncation from HANDLE to long
#endif

// #define FULLSCREEN

#include "template.h"
#include "camera.h"
#include "game.h"
#include "physics.h"
#include "renderer.h"
#include "surface.h"
#include <corecrt_math.h>
#include <cstdio>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#define WIN32_LEAN_AND_MEAN
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
  // NOTE: changed this from 1000. / double(...)
  // Multiplying by 1000 here led to delta times above 1 sec for 240 frames a
  // second (anyway, logging would show like 242 when less then a second had
  // passed)
  // This also fixes massive performance issues with the physics update, which
  // was called way too many times a second (6000?) as a result
  inv_freq = 1. / double(f.QuadPart);
}

// Math Stuff
// ----------------------------------------------------------------------------
const quat quat::Identity(0.0f, 0.0f, 0.0f, 1.0f);
const vec3 vec3::zero = vec3(0.0f);
const vec3 vec3::right = vec3(0.0f, 1.0f, 0.0f);
const vec3 vec3::up = vec3(0.0f, 0.0f, 1.0f);
const vec3 vec3::forward = vec3(1.0f, 0.0f, 0.0f);
vec3::vec3(vec4 v) : x(v.x), y(v.y), z(v.z) {}
vec3 normalize(const vec3 &v) { return v.normalized(); }
vec3 cross(const vec3 &a, const vec3 &b) { return a.cross(b); }
float dot(const vec3 &a, const vec3 &b) { return a.dot(b); }
vec3 operator*(const float &s, const vec3 &v) {
  return {v.x * s, v.y * s, v.z * s};
}
vec3 operator*(const vec3 &v, const float &s) {
  return {v.x * s, v.y * s, v.z * s};
}
vec3 vec3::transform(const vec3 &v, const quat &q) {
  // v + 2.0*cross(q.xyz, cross(q.xyz,v) + q.w*v);
  vec3 qv(q.x, q.y, q.z);
  vec3 retVal = v;
  retVal += 2.0f * qv.cross(qv.cross(v) + q.w * v);
  return retVal;
}

vec4 operator*(const float &s, const vec4 &v) {
  return {v.x * s, v.y * s, v.z * s, v.w * s};
}
vec4 operator*(const vec4 &v, const float &s) {
  return {v.x * s, v.y * s, v.z * s, v.w * s};
}

vec4 operator*(const vec4 &v, const mat4 &M) {
  vec4 mx(M.cell[0], M.cell[1], M.cell[2], M.cell[3]);
  vec4 my(M.cell[4], M.cell[5], M.cell[6], M.cell[7]);
  vec4 mz(M.cell[8], M.cell[9], M.cell[10], M.cell[11]);
  vec4 mw(M.cell[12], M.cell[13], M.cell[14], M.cell[15]);
  return v.x * mx + v.y * my + v.z * mz + v.w * mw;
}

vec4 mat4::operator*(const vec4 &v) {
  vec4 mx(cell[0], cell[4], cell[8], cell[12]);
  vec4 my(cell[1], cell[5], cell[9], cell[13]);
  vec4 mz(cell[2], cell[6], cell[10], cell[14]);
  vec4 mw(cell[3], cell[7], cell[11], cell[15]);
  return v.x * mx + v.y * my + v.z * mz + v.w * mw;
}

mat4::mat4() {
  memset(cell, 0, 64);
  cell[0] = cell[5] = cell[10] = cell[15] = 1.0f;
}

mat4::mat4(float inMat[4][4]) { memcpy(mat, inMat, 16 * sizeof(float)); }

mat4 mat4::identity() {
  mat4 r;
  memset(r.cell, 0, 64);
  r.cell[0] = r.cell[5] = r.cell[10] = r.cell[15] = 1.0f;
  return r;
}

mat4 mat4::rotate(const vec3 l, const float a) {
  // http://inside.mines.edu/fs_home/gmurray/ArbitraryAxisRotation
  mat4 M;
  const float u = l.x, v = l.y, w = l.z, ca = cosf(a), sa = sinf(a);
  M.cell[0] = u * u + (v * v + w * w) * ca,
  M.cell[1] = u * v * (1 - ca) - w * sa;
  M.cell[2] = u * w * (1 - ca) + v * sa, M.cell[4] = u * v * (1 - ca) + w * sa;
  M.cell[5] = v * v + (u * u + w * w) * ca,
  M.cell[6] = v * w * (1 - ca) - u * sa;
  M.cell[8] = u * w * (1 - ca) - v * sa, M.cell[9] = v * w * (1 - ca) + u * sa;
  M.cell[10] = w * w + (u * u + v * v) * ca;
  M.cell[3] = M.cell[7] = M.cell[11] = M.cell[12] = M.cell[13] = M.cell[14] = 0,
  M.cell[15] = 1;
  return M;
}

mat4 mat4::rotatex(const float rad) {
  mat4 M;
  const float ca = cosf(rad), sa = sinf(rad);
  M.cell[5] = ca, M.cell[6] = sa;
  M.cell[9] = -sa, M.cell[10] = ca;
  return M;
}

mat4 mat4::rotatey(const float rad) {
  mat4 M;
  const float ca = cosf(rad), sa = sinf(rad);
  M.cell[0] = ca, M.cell[2] = -sa;
  M.cell[8] = sa, M.cell[10] = ca;
  return M;
}

mat4 mat4::rotatez(const float rad) {
  mat4 M;
  const float ca = cosf(rad), sa = sinf(rad);
  M.cell[0] = ca, M.cell[1] = sa;
  M.cell[4] = -sa, M.cell[5] = ca;
  return M;
}

// SOURCE:
// https://qiita.com/aa_debdeb/items/3d02e28fb9ebfa357eaf#%E3%82%AF%E3%82%A9%E3%83%BC%E3%82%BF%E3%83%8B%E3%82%AA%E3%83%B3%E3%81%8B%E3%82%89%E5%9B%9E%E8%BB%A2%E8%A1%8C%E5%88%97
mat4 mat4::CreateFromQuaternion(const class quat &q) {
  float mat[4][4];

  mat[0][0] = 2.0f * q.w * q.w + 2.0f * q.x * q.x - 1.0f;
  mat[0][1] = 2.0f * q.x * q.y - 2.0f * q.w * q.z;
  mat[0][2] = 2.0f * q.x * q.z + 2.0f * q.w * q.y;
  mat[0][3] = 0.0f;

  mat[1][0] = 2.0f * q.x * q.y + 2.0f * q.w * q.z;
  mat[1][1] = 2.0f * q.w * q.w + 2.0f * q.y * q.y - 1.0f;
  mat[1][2] = 2.0f * q.y * q.z - 2.0f * q.w * q.x;
  mat[1][3] = 0.0f;

  mat[2][0] = 2.0f * q.x * q.z - 2.0f * q.w * q.y;
  mat[2][1] = 2.0f * q.y * q.z + 2.0f * q.w * q.x;
  mat[2][2] = 2.0f * q.w * q.w + 2.0f * q.z * q.z - 1.0f;
  mat[2][3] = 0.0f;

  mat[3][0] = 0.0f;
  mat[3][1] = 0.0f;
  mat[3][2] = 0.0f;
  mat[3][3] = 1.0f;

  return {mat};
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
static bool firstframe = true;

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

  shared_ptr<Surface> surface =
      std::make_shared<Surface>(ScreenWidth, ScreenHeight);

  shared_ptr<FollowCamera> camera =
      std::make_shared<FollowCamera>(vec3(0, 0, 2), vec3(7, 0, 0), vec3::up);
  unique_ptr<Renderer> renderer = std::make_unique<Renderer>(camera, surface);
  shared_ptr<Entities> entities = std::make_shared<Entities>();

  game = new Game();
  game->SetTarget(surface);
  game->SetCamera(camera);
  game->SetEntities(entities);

  ShowCursor(false);

  timer t;
  t.reset();

  float tPhysics = 0.0;
  float physicsTimeAccumulator = 0.0;

  while (!exitapp) {
    if (firstframe) {
      game->Init();
      firstframe = false;
      // HACK: Only make game start one init is over.
      // This is to prevent the physics update from occuring many times before
      // anything is even shown.
      t.reset();
    }

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
    camera->update(elapsedTime, entities->ProvideCameraFollow());
    renderer->Draw3D(elapsedTime, camera, entities->GetStaticEntities(),
                     entities->GetDynamicEntities());

    game->SetupKeys();

    // event loop
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        exitapp = 1;
        break;
      case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_ESCAPE) {
          exitapp = 1;
          // find other keys here: http://sdl.beuc.net/sdl.wiki/SDLKey
        }
        game->KeyDown(event.key.keysym.scancode);
        break;
      case SDL_KEYUP:
        game->KeyUp(event.key.keysym.scancode);
        break;
      case SDL_MOUSEMOTION:
        game->MouseMove(event.motion.x, event.motion.y);
        break;
      case SDL_MOUSEBUTTONUP:
        game->MouseUp(event.button.button);
        break;
      case SDL_MOUSEBUTTONDOWN:
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
