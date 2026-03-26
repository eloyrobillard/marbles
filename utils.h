#pragma once

#include "pch.h"
#include "template.h"

using std::copy;
using std::ostream;
using std::ostream_iterator;
using std::vector;
using Tmpl8::vec3;

ostream &operator<<(ostream &os, const vec3 &v) {
  return os << "{ " << v.x << ", " << v.y << ", " << v.z << " }";
}

template <typename T> ostream &operator<<(ostream &os, const vector<T> &v) {
  copy(v.begin(), v.end(), ostream_iterator<T>(os, ", "));
  return os;
}
