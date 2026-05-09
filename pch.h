#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_video.h>
#include <SDL3_ttf/SDL_ttf.h>
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

typedef unsigned char uchar;
typedef unsigned char byte;
typedef __int64 int64;
typedef unsigned __int64 uint64;
typedef unsigned int uint;

using namespace std::views;

using rapidjson::Value;
using std::cerr;
using std::cout;
using std::deque;
using std::endl;
using std::optional;
using std::ostream;
using std::ostream_iterator;
using std::pair;
using std::shared_ptr;
using std::stack;
using std::string;
using std::thread;
using std::tuple;
using std::unique_ptr;
using std::vector;

#define ALL(v) (v).begin(), (v).end()

// NOTE: commenting this out because it's giving me trouble on first start of a
// copy of the project
// template <typename T> ostream &operator<<(ostream &os,
// const vector<T> &v) {
//   copy(ALL(v), ostream_iterator<T>(os, ", "));
//   return os;
// }
