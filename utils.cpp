#include "pch.h"

// FIX: variadic 引数を渡すの問題：https://stackoverflow.com/a/55676112
// template <class... Args>
// auto throttle(i64 delay_ms, std::function<void(Args...)> &&f) {
//   static auto prev = 0;
//   return [&, delay_ms](Args &&...args) {
//     auto now = clock();
//     if (now - prev >= delay_ms) {
//       f(args...);
//       prev = now;
//     }
//   };
// }

// SOURCE: https://stackoverflow.com/a/55676112
template <class T>
auto throttle(i64 delay_ms, std::function<void(T)> &&f) {
  static auto prev = 0;
  return [&, delay_ms](T t) {
    auto now = clock();
    if (now - prev >= delay_ms) {
      f(t);
      prev = now;
    }
  };
}

template <class T, class U>
auto throttle(i64 delay_ms, std::function<void(T, U)> &&f) {
  static auto prev = 0;
  return [&, delay_ms](T t, U u) {
    auto now = clock();
    if (now - prev >= delay_ms) {
      f(t, u);
      prev = now;
    }
  };
}

template <class T, class U, class V>
auto throttle(i64 delay_ms, std::function<void(T, U, V)> &&f) {
  static auto prev = 0;
  return [&, delay_ms](T t, U u, V v) {
    auto now = clock();
    if (now - prev >= delay_ms) {
      f(t, u, v);
      prev = now;
    }
  };
}

template <class T, class U, class V, class W>
auto throttle(i64 delay_ms, std::function<void(T, U, V, W)> &&f) {
  static auto prev = 0;
  return [&, delay_ms](T t, U u, V v, W w) {
    auto now = clock();
    if (now - prev >= delay_ms) {
      f(t, u, v, w);
      prev = now;
    }
  };
}

template <class T, class U, class V, class W, class X>
auto throttle(i64 delay_ms, std::function<void(T, U, V, W, X)> &&f) {
  static auto prev = 0;
  return [&, delay_ms](T t, U u, V v, W w, X x) {
    auto now = clock();
    if (now - prev >= delay_ms) {
      f(t, u, v, w, x);
      prev = now;
    }
  };
}
