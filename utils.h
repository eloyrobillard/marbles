#ifndef _UTILS_H

#include "pch.h"

template <class T>
auto throttle(i64 delay_ms, std::function<void(T)> &&f);
template <class T, class U>
auto throttle(i64 delay_ms, std::function<void(T, U)> &&f);
template <class T, class U, class V>
auto throttle(i64 delay_ms, std::function<void(T, U, V)> &&f);
template <class T, class U, class V, class W>
auto throttle(i64 delay_ms, std::function<void(T, U, V, W)> &&f);
template <class T, class U, class V, class W, class X>
auto throttle(i64 delay_ms, std::function<void(T, U, V, W, X)> &&f);
#endif
