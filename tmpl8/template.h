// Template, BUAS version https://www.buas.nl/games
// IGAD/BUAS(NHTV)/UU - Jacco Bikker - 2006-2020

#pragma once
#if defined _M_IX86
#pragma comment(                                                               \
    linker,                                                                    \
    "/manifestdependency:\"name='dlls_x86' version='1.0.0.0' type='win32'\"")
#elif defined _M_X64
#pragma comment(                                                               \
    linker,                                                                    \
    "/manifestdependency:\"name='dlls_x64' version='1.0.0.0' type='x64'\"")
#endif

#include "pch.h"

constexpr int ScreenWidth = 1080;
constexpr int ScreenHeight = 720;
// #define FULLSCREEN
#define ADVANCEDGL // faster if your system supports it. Switches SDL2's texture
                   // buffer out for OpenGL texture buffer with mappings to CPU
                   // Memory.

static const char *TemplateVersion = "Template_v2019.08";

// SIMD Intrinsics headers.
// #include "emmintrin.h"
// #include "immintrin.h"

int filesize(FILE *f);
#define MALLOC64(x) _aligned_malloc(x, 64)
#define FREE64(x) _aligned_free(x)

namespace Tmpl8 {

#define PREFETCH(x) _mm_prefetch((const char *)(x), _MM_HINT_T0)
#define PREFETCH_ONCE(x) _mm_prefetch((const char *)(x), _MM_HINT_NTA)
#define PREFETCH_WRITE(x) _m_prefetchw((const char *)(x))
#define loadss(mem) _mm_load_ss((const float *const)(mem))
#define broadcastps(ps) _mm_shuffle_ps((ps), (ps), 0)
#define broadcastss(ss) broadcastps(loadss((ss)))

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define likely(expr) (expr)
#define unlikely(expr) (expr)
#else
#define likely(expr) __builtin_expect((expr), true)
#define unlikely(expr) __builtin_expect((expr), false)
#endif

struct timer {
  typedef long long value_type;
  static double inv_freq;
  value_type start;
  timer();
  float elapsed() const;
  static value_type get();
  static double to_time(const value_type vt);
  void reset();
  static void init();
};

inline bool BadFloat(float x) {
  return ((*reinterpret_cast<uint *>(&x) & 0x7f000000) == 0x7f000000);
}

}; // namespace Tmpl8
