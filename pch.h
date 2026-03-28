#pragma once
//------------------------------------------------------------
//! @file pch.h
//! @brief プリコンパイル済みヘッダーファイル
//! @author つきの
//------------------------------------------------------------
#include <SDL_log.h>
#include <SDL_scancode.h>
#include <SOIL/include/SOIL.h>
#include <algorithm>
#include <bitset>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <fstream>
#include <glew.h>
#include <iostream>
#include <iterator>
#include <memory>
#include <ostream>
#include <rapidjson/document.h>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using rapidjson::Value;
using std::cerr;
using std::cout;
using std::endl;
using std::ostream;
using std::ostream_iterator;
using std::string;
using std::vector;

template <typename T> ostream &operator<<(ostream &os, const vector<T> &v) {
  copy(v.begin(), v.end(), ostream_iterator<T>(os, ", "));
  return os;
}
