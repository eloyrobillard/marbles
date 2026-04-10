#pragma once

#include <SDL_log.h>
#include <SDL_scancode.h>
#include <SOIL/include/SOIL.h>
#include <algorithm>
#include <bitset>
#include <cassert>
#include <cmath>
#include <concepts>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <fstream>
#include <glew.h>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <optional>
#include <ostream>
#include <print>
#include <ranges>
#include <rapidjson/document.h>
#include <sstream>
#include <stack>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

using namespace std::views;

using rapidjson::Value;
using std::cerr;
using std::cout;
using std::endl;
using std::optional;
using std::ostream;
using std::ostream_iterator;
using std::pair;
using std::stack;
using std::string;
using std::thread;
using std::tuple;
using std::unique_ptr;
using std::vector;

#define v(T) vector<T>
#define vv(T) v(v(T))
#define ALL(v) (v).begin(), (v).end()

template <typename T> ostream &operator<<(ostream &os, const vector<T> &v) {
  copy(v.begin(), v.end(), ostream_iterator<T>(os, ", "));
  return os;
}
