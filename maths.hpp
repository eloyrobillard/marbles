#ifndef _MATHS_HPP
#define _MATHS_HPP

#include "pch.h"

namespace Maths {
template <typename T>
constexpr T Min(T a, T b) {
  return (a > b) ? b : a;
}

template <typename T>
constexpr T Max(T a, T b) {
  return (a > b) ? a : b;
}

template <typename T>
constexpr T Clamp(T value, T min, T max) {
  return Min(max, Max(value, min));
}

constexpr f32 PI =
    3.14159265358979323846264338327950288419716939937510582097494459072381640628620899862803482534211706798f;
constexpr f32 TAU =
    6.2831853071795864769252867665590057683943387987502116419498891846156328125724179972560696506842341359642961730265646132941876892f;

const f32 RadToDeg = 360.0f / TAU;
const f32 DegToRad = TAU / 360.0f;

inline f32 Rand(f32 range) {
  return ((f32)rand() / RAND_MAX) * range;
}
inline int IRand(int range) {
  return rand() % range;
}

class quat;
class vec4;

// vectors
class vec2 // adapted from https://github.com/dcow/RayTracer
{
public:
  union {
    struct {
      f32 x, y;
    };
    f32 cell[2];
  };
  vec2() {}
  explicit vec2(f32 v) : x(v), y(v) {}
  vec2(f32 x, f32 y) : x(x), y(y) {}
  vec2 operator-() const { return {-x, -y}; }
  vec2 operator+(const vec2 &addOperand) const {
    return {x + addOperand.x, y + addOperand.y};
  }
  vec2 operator-(const vec2 &operand) const {
    return {x - operand.x, y - operand.y};
  }
  vec2 operator*(const vec2 &operand) const {
    return {x * operand.x, y * operand.y};
  }
  vec2 operator*(f32 operand) const { return {x * operand, y * operand}; }
  void operator-=(const vec2 &a) {
    x -= a.x;
    y -= a.y;
  }
  void operator+=(const vec2 &a) {
    x += a.x;
    y += a.y;
  }
  void operator*=(const vec2 &a) {
    x *= a.x;
    y *= a.y;
  }
  void operator*=(f32 a) {
    x *= a;
    y *= a;
  }
  f32 &operator[](const int idx) { return cell[idx]; }
  f32 length() { return sqrtf(x * x + y * y); }
  f32 sqrLentgh() { return x * x + y * y; }
  vec2 normalized() {
    f32 r = 1.0f / length();
    return {x * r, y * r};
  }
  void normalize() {
    f32 r = 1.0f / length();
    x *= r;
    y *= r;
  }
  static vec2 normalize(vec2 v) { return v.normalized(); }
  [[nodiscard]] f32 dot(const vec2 &operand) const {
    return x * operand.x + y * operand.y;
  }
};

class vec3 {
public:
  union {
    struct {
      f32 x, y, z, dummy;
    };
    f32 cell[4];
  };

  vec3() : x(0.0f), y(0.0f), z(0.0f) {}
  explicit vec3(f32 v) : x(v), y(v), z(v) {}
  explicit vec3(const vec4 &v);
  explicit vec3(const quat &q);
  vec3(f32 x, f32 y, f32 z) : x(x), y(y), z(z) {}
  vec3 operator/(f32 f) const { return {x / f, y / f, z / f}; }
  vec3 operator-() const { return {-x, -y, -z}; }
  vec3 operator+(const vec3 &addOperand) const {
    return {x + addOperand.x, y + addOperand.y, z + addOperand.z};
  }
  vec3 operator-(const vec3 &operand) const {
    return {x - operand.x, y - operand.y, z - operand.z};
  }
  vec3 operator*(const vec3 &operand) const {
    return {x * operand.x, y * operand.y, z * operand.z};
  }
  void operator-=(const vec3 &a) {
    x -= a.x;
    y -= a.y;
    z -= a.z;
  }
  void operator+=(const vec3 &a) {
    x += a.x;
    y += a.y;
    z += a.z;
  }
  void operator*=(const vec3 &a) {
    x *= a.x;
    y *= a.y;
    z *= a.z;
  }

  void operator*=(const f32 a) {
    x *= a;
    y *= a;
    z *= a;
  }

  static vec3 rand(f32 xRange, f32 yRange, f32 zRange) {
    return {Rand(xRange), Rand(yRange), Rand(zRange)};
  }

  static vec3 average(const vector<vec3> &vs) {
    vec3 res = vec3(0.0f);

    for (const auto &v : vs) {
      res += v;
    }

    return res / static_cast<f32>(vs.size());
  }

  f32 operator[](const uint &idx) const { return cell[idx]; }
  f32 &operator[](const uint &idx) { return cell[idx]; }
  [[nodiscard]] f32 length() const { return sqrtf(x * x + y * y + z * z); }
  [[nodiscard]] f32 sqrLentgh() const { return x * x + y * y + z * z; }
  [[nodiscard]] vec3 normalized() const {
    f32 len = length();

    if (len != 0.0f) {
      f32 r = 1.0f / len;
      return {x * r, y * r, z * r};
    }

    return vec3::zero;
  }
  void normalize() {
    f32 len = length();

    if (len != 0.0f) {
      f32 r = 1.0f / length();
      x *= r;
      y *= r;
      z *= r;
    } else {
      x = 0.0f;
      y = 0.0f;
      z = 0.0f;
    }
  }

  static vec3 normalize(const vec3 v) { return v.normalized(); }

  [[nodiscard]] vec3 cross(const vec3 &operand) const {
    return {y * operand.z - z * operand.y, z * operand.x - x * operand.z,
            x * operand.y - y * operand.x};
  }

  [[nodiscard]] f32 dot(const vec3 &operand) const {
    return x * operand.x + y * operand.y + z * operand.z;
  }

  [[nodiscard]] f32 distanceSqrd(const vec3 &operand) const {
    const vec3 diff = {x - operand.x, y - operand.y, z - operand.z};
    return diff.dot(diff);
  }

  [[nodiscard]] f32 distance(const vec3 &operand) const {
    const vec3 diff = {x - operand.x, y - operand.y, z - operand.z};
    return sqrt(diff.dot(diff));
  }

  static vec3 transform(const vec3 &vec, const quat &q);

  friend ostream &operator<<(ostream &os, const vec3 &v) {
    return os << "{ " << v.x << ", " << v.y << ", " << v.z << " }";
  }

  static const vec3 zero;
  static const vec3 right;
  static const vec3 up;
  static const vec3 forward;
};

class vec4 {
public:
  union {
    struct {
      f32 x, y, z, w;
    };
    struct {
      vec3 xyz;
      f32 w2;
    };
    f32 cell[4];
  };

  vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
  explicit vec4(f32 v) : x(v), y(v), z(v), w(v) {}
  vec4(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {}

  explicit vec4(vec3 v) : x(v.x), y(v.y), z(v.z), w(0.0f) {}
  vec4(vec3 a, f32 b) : x(a.x), y(a.y), z(a.z), w(b) {}
  vec4 operator-() const { return vec4(-x, -y, -z, -w); }
  vec4 operator+(const vec4 &addOperand) const {
    return vec4(x + addOperand.x, y + addOperand.y, z + addOperand.z,
                w + addOperand.w);
  }
  vec4 operator-(const vec4 &operand) const {
    return vec4(x - operand.x, y - operand.y, z - operand.z, w - operand.w);
  }
  vec4 operator*(const vec4 &operand) const {
    return vec4(x * operand.x, y * operand.y, z * operand.z, w * operand.w);
  }
  void operator-=(const vec4 &a) {
    x -= a.x;
    y -= a.y;
    z -= a.z;
    w -= a.w;
  }
  void operator+=(const vec4 &a) {
    x += a.x;
    y += a.y;
    z += a.z;
    w += a.w;
  }
  void operator*=(const vec4 &a) {
    x *= a.x;
    y *= a.y;
    z *= a.z;
    w *= a.w;
  }
  void operator*=(f32 a) {
    x *= a;
    y *= a;
    z *= a;
    w *= a;
  }
  f32 &operator[](const int idx) { return cell[idx]; }
  f32 operator[](const uint &idx) const { return cell[idx]; }
  f32 length() { return sqrtf(x * x + y * y + z * z + w * w); }
  f32 sqrLentgh() { return x * x + y * y + z * z + w * w; }
  vec4 normalized() {
    f32 r = 1.0f / length();
    return vec4(x * r, y * r, z * r, w * r);
  }
  void normalize() {
    f32 r = 1.0f / length();
    x *= r;
    y *= r;
    z *= r;
    w *= r;
  }
  static vec4 normalize(vec4 v) { return v.normalized(); }
  f32 dot(const vec4 &operand) const {
    return x * operand.x + y * operand.y + z * operand.z + w * operand.w;
  }

  friend ostream &operator<<(ostream &os, const vec4 v) {
    return os << "{ " << v.x << ", " << v.y << ", " << v.z << ", " << v.w
              << " }";
  }
};

vec3 normalize(const vec3 &v);
vec3 cross(const vec3 &a, const vec3 &b);
f32 dot(const vec3 &a, const vec3 &b);
vec3 operator*(const f32 &s, const vec3 &v);
vec3 operator*(const vec3 &v, const f32 &s);
vec4 operator*(const f32 &s, const vec4 &v);
vec4 operator*(const vec4 &v, const f32 &s);

struct Ray {
  vec3 origin;
  vec3 direction;
};

class uint4 {
public:
  union {
    struct {
      uint x, y, z, w;
    };
    uint cell[4];
  };
  uint4() {}
  uint4(int v) : x(v), y(v), z(v), w(v) {}
  uint4(int x, int y, int z, int w) : x(x), y(y), z(z), w(w) {}
  uint4 operator+(const uint4 &addOperand) const {
    return uint4(x + addOperand.x, y + addOperand.y, z + addOperand.z,
                 w + addOperand.w);
  }
  uint4 operator-(const uint4 &operand) const {
    return uint4(x - operand.x, y - operand.y, z - operand.z, w - operand.w);
  }
  uint4 operator*(const uint4 &operand) const {
    return uint4(x * operand.x, y * operand.y, z * operand.z, w * operand.w);
  }
  uint4 operator*(uint operand) const {
    return uint4(x * operand, y * operand, z * operand, w * operand);
  }
  void operator-=(const uint4 &a) {
    x -= a.x;
    y -= a.y;
    z -= a.z;
    w -= a.w;
  }
  void operator+=(const uint4 &a) {
    x += a.x;
    y += a.y;
    z += a.z;
    w += a.w;
  }
  void operator*=(const uint4 &a) {
    x *= a.x;
    y *= a.y;
    z *= a.z;
    w *= a.w;
  }
  void operator*=(uint a) {
    x *= a;
    y *= a;
    z *= a;
    w *= a;
  }
  uint &operator[](const int idx) { return cell[idx]; }
};

class int4 {
public:
  union {
    struct {
      int x, y, z, w;
    };
    int cell[4];
  };
  int4() {}
  int4(int v) : x(v), y(v), z(v), w(v) {}
  int4(int x, int y, int z, int w) : x(x), y(y), z(z), w(w) {}
  int4 operator-() const { return int4(-x, -y, -z, -w); }
  int4 operator+(const int4 &addOperand) const {
    return int4(x + addOperand.x, y + addOperand.y, z + addOperand.z,
                w + addOperand.w);
  }
  int4 operator-(const int4 &operand) const {
    return int4(x - operand.x, y - operand.y, z - operand.z, w - operand.w);
  }
  int4 operator*(const int4 &operand) const {
    return int4(x * operand.x, y * operand.y, z * operand.z, w * operand.w);
  }
  int4 operator*(int operand) const {
    return int4(x * operand, y * operand, z * operand, w * operand);
  }
  void operator-=(const int4 &a) {
    x -= a.x;
    y -= a.y;
    z -= a.z;
    w -= a.w;
  }
  void operator+=(const int4 &a) {
    x += a.x;
    y += a.y;
    z += a.z;
    w += a.w;
  }
  void operator*=(const int4 &a) {
    x *= a.x;
    y *= a.y;
    z *= a.z;
    w *= a.w;
  }
  void operator*=(int a) {
    x *= a;
    y *= a;
    z *= a;
    w *= a;
  }
  int &operator[](const int idx) { return cell[idx]; }
};

inline f32 lerp(const f32 a, const f32 b, const f32 f) {
  return a + f * (b - a);
}
inline vec3 lerp(const vec3 &a, const vec3 &b, const f32 f) {
  return a + f * (b - a);
}

// NOTE: From "Game Programming in C++" by Sanjay Madhav
class quat {
public:
  f32 x;
  f32 y;
  f32 z;
  f32 w;

  quat() { *this = quat::Identity; }

  // This directly sets the quaternion components --
  // don't use for axis/angle
  explicit quat(f32 inX, f32 inY, f32 inZ, f32 inW) { Set(inX, inY, inZ, inW); }

  // Construct the quaternion from an axis and angle
  // It is assumed that axis is already normalized,
  // and the angle is in radians
  explicit quat(const vec3 &axis, f32 angle) {
#ifdef _DEBUG
    auto axisLen = axis.length();
    assert(axisLen > 0.95 && axisLen < 1.05);
#endif
    f32 scalar = sin(angle / 2.0f);
    x = axis.x * scalar;
    y = axis.y * scalar;
    z = axis.z * scalar;
    w = cos(angle / 2.0f);
  }

  // Directly set the internal components
  void Set(f32 inX, f32 inY, f32 inZ, f32 inW) {
    x = inX;
    y = inY;
    z = inZ;
    w = inW;
  }

  static quat Conjugate(const quat &q) { return quat{-q.x, -q.y, -q.z, q.w}; }

  void Conjugate() {
    x *= -1.0f;
    y *= -1.0f;
    z *= -1.0f;
  }

  f32 LengthSq() const { return (x * x + y * y + z * z + w * w); }

  f32 Length() const { return sqrt(LengthSq()); }

  void Normalize() {
    f32 length = Length();
    x /= length;
    y /= length;
    z /= length;
    w /= length;
  }

  [[nodiscard]] quat Normalized() const {
    f32 len = Length();
    return quat{x / len, y / len, z / len, w / len};
  }

  // Normalize the provided quaternion
  static quat Normalize(const quat &q) {
    quat retVal = q;
    retVal.Normalize();
    return retVal;
  }

  // Linear interpolation
  static quat Lerp(const quat &a, const quat &b, f32 f) {
    quat retVal;
    retVal.x = lerp(a.x, b.x, f);
    retVal.y = lerp(a.y, b.y, f);
    retVal.z = lerp(a.z, b.z, f);
    retVal.w = lerp(a.w, b.w, f);
    retVal.Normalize();
    return retVal;
  }

  static f32 Dot(const quat &a, const quat &b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
  }

  // Spherical Linear Interpolation
  static quat Slerp(const quat &a, const quat &b, f32 f) {
    f32 rawCosm = quat::Dot(a, b);

    f32 cosom = -rawCosm;
    if (rawCosm >= 0.0f) {
      cosom = rawCosm;
    }

    f32 scale0, scale1;

    if (cosom < 0.9999f) {
      const f32 omega = acos(cosom);
      const f32 invSin = 1.f / sin(omega);
      scale0 = sin((1.f - f) * omega) * invSin;
      scale1 = sin(f * omega) * invSin;
    } else {
      // Use linear interpolation if the quaternions
      // are collinear
      scale0 = 1.0f - f;
      scale1 = f;
    }

    if (rawCosm < 0.0f) {
      scale1 = -scale1;
    }

    quat retVal;
    retVal.x = scale0 * a.x + scale1 * b.x;
    retVal.y = scale0 * a.y + scale1 * b.y;
    retVal.z = scale0 * a.z + scale1 * b.z;
    retVal.w = scale0 * a.w + scale1 * b.w;
    retVal.Normalize();
    return retVal;
  }

  // NOTE: From "Game Engine Architecture (4 ed)" by Jason Gregory
  static vec3 RotateVector(const quat &q, const vec3 &v) {
    quat vecQuat(v.x, v.y, v.z, 0);
    quat qv = Concatenate(q, vecQuat);
    quat qvq = Concatenate(qv, Conjugate(q));
    return vec3{qvq};
  }

  // NOTE: From "Game Programming in C++" by Sanjay Madhav
  // Concatenate
  // Rotate by q FOLLOWED BY p
  static quat Concatenate(const quat &q, const quat &p) {
    quat retVal;

    // Vector component is:
    // ps * qv + qs * pv + pv x qv
    vec3 qv(q.x, q.y, q.z);
    vec3 pv(p.x, p.y, p.z);
    vec3 newVec = p.w * qv + q.w * pv + pv.cross(qv);
    retVal.x = newVec.x;
    retVal.y = newVec.y;
    retVal.z = newVec.z;

    // Scalar component is:
    // ps * qs - pv . qv
    retVal.w = p.w * q.w - pv.dot(qv);

    return retVal;
  }

  friend quat operator*(const quat &q, const quat &p) {
    return quat(q.w * p.x - q.z * p.y + q.y * p.z + q.x * p.w,
                q.z * p.x + q.w * p.y - q.x * p.z + q.y * p.w,
                -q.y * p.x + q.x * p.y + q.w * p.z + q.z * p.w,
                -q.x * p.x - q.y * p.y - q.z * p.z + q.w * p.w);
  }

  friend std::ostream &operator<<(std::ostream &os, const quat &q) {
    os << '[' << q.x << ", " << q.y << ", " << q.z << ", " << q.w << ']';
    return os;
  }

  void operator*=(const quat &q) {
    x = w * q.x - z * q.y + y * q.z + x * q.w;
    y = z * q.x + w * q.y - x * q.z + y * q.w;
    z = -y * q.x + x * q.y + w * q.z + z * q.w;
    w = -x * q.x - y * q.y - z * q.z + w * q.w;
  }

  static const quat Identity;
};

class mat4 {
public:
  mat4();
  mat4(f32[4][4]);
  union {
    f32 cell[16];
    f32 mat[4][4];
  };

  friend std::ostream &operator<<(std::ostream &os, const mat4 &m) {
    os << m.mat[0][0] << ' ' << m.mat[0][1] << ' ' << m.mat[0][2] << ' '
       << m.mat[0][3] << endl;
    os << m.mat[1][0] << ' ' << m.mat[1][1] << ' ' << m.mat[1][2] << ' '
       << m.mat[1][3] << endl;
    os << m.mat[2][0] << ' ' << m.mat[2][1] << ' ' << m.mat[2][2] << ' '
       << m.mat[2][3] << endl;
    os << m.mat[3][0] << ' ' << m.mat[3][1] << ' ' << m.mat[3][2] << ' '
       << m.mat[3][3] << endl;
    return os;
  }

  f32 &operator[](const int idx) { return cell[idx]; }
  vec4 operator*(const vec4 &v);
  static mat4 identity();

  static mat4 rotate(vec3 v, f32 rad);
  static mat4 rotatex(f32 rad);
  static mat4 rotatey(f32 rad);
  static mat4 rotatez(f32 rad);

  // NOTE: From "Game Programming in C++" by Sanjay Madhav
  static mat4 CreateFromQuaternion(const class quat &q);
  static mat4 CreateTranslation(const vec3 &trans) {
    f32 temp[4][4] = {{1.0f, 0.0f, 0.0f, 0.0f},
                      {0.0f, 1.0f, 0.0f, 0.0f},
                      {0.0f, 0.0f, 1.0f, 0.0f},
                      {trans.x, trans.y, trans.z, 1.0f}};
    return {temp};
  }

  static mat4 RemoveTranslation(const mat4 &m) {
    mat4 mm{m};
    mm.mat[0][3] = mm.mat[1][3] = mm.mat[2][3] = mm.mat[3][0] = mm.mat[3][1] =
        mm.mat[3][2] = 0.0f;
    return mm;
  }

  static mat4 CreateScale(float xScale, float yScale, float zScale) {
    float temp[4][4] = {{xScale, 0.0f, 0.0f, 0.0f},
                        {0.0f, yScale, 0.0f, 0.0f},
                        {0.0f, 0.0f, zScale, 0.0f},
                        {0.0f, 0.0f, 0.0f, 1.0f}};
    return {temp};
  }
  static mat4 CreateScale(const vec3 &v) { return CreateScale(v.x, v.y, v.z); }
  static mat4 CreateScale(f32 scale) {
    return CreateScale(scale, scale, scale);
  }

  static mat4 CreatePerspectiveFOV(f32 fovY, f32 width, f32 height, f32 near,
                                   f32 far) {
    f32 yScale = 1.0f / tan(fovY / 2.0f);
    f32 xScale = yScale * height / width;
    f32 temp[4][4] = {{xScale, 0.0f, 0.0f, 0.0f},
                      {0.0f, yScale, 0.0f, 0.0f},
                      {0.0f, 0.0f, far / (far - near), 1.0f},
                      {0.0f, 0.0f, -near * far / (far - near), 0.0f}};
    return {temp};
  }

  static mat4 CreateOrtho(f32 width, f32 height, f32 near, f32 far) {
    f32 temp[4][4] = {{2.0f / width, 0.0f, 0.0f, 0.0f},
                      {0.0f, 2.0f / height, 0.0f, 0.0f},
                      {0.0f, 0.0f, 1.0f / (far - near), 0.0f},
                      {0.0f, 0.0f, near / (near - far), 1.0f}};
    return {temp};
  }

  static mat4 CreateLookAt(const vec3 &eye, const vec3 &target,
                           const vec3 &up) {
    vec3 zaxis = vec3::normalize(target - eye);
    vec3 xaxis = vec3::normalize(up.cross(zaxis));
    vec3 yaxis = vec3::normalize(zaxis.cross(xaxis));
    vec3 trans;
    trans.x = -xaxis.dot(eye);
    trans.y = -yaxis.dot(eye);
    trans.z = -zaxis.dot(eye);

    float temp[4][4] = {{xaxis.x, yaxis.x, zaxis.x, 0.0f},
                        {xaxis.y, yaxis.y, zaxis.y, 0.0f},
                        {xaxis.z, yaxis.z, zaxis.z, 0.0f},
                        {trans.x, trans.y, trans.z, 1.0f}};
    return {temp};
  }

  static mat4 CreateLookAtSkybox(const vec3 &eye, const vec3 &target,
                                 const vec3 &up) {
    vec3 zaxis = vec3::normalize(target - eye);
    vec3 xaxis = vec3::normalize(up.cross(zaxis));
    vec3 yaxis = vec3::normalize(zaxis.cross(xaxis));
    vec3 trans;
    trans.x = -xaxis.dot(eye);
    trans.y = -yaxis.dot(eye);
    trans.z = -zaxis.dot(eye);

    f32 temp[4][4] = {{xaxis.y, yaxis.y, zaxis.y, 0.0f},
                      {xaxis.z, yaxis.z, zaxis.z, 0.0f},
                      {xaxis.x, yaxis.x, zaxis.x, 0.0f},
                      {0.0f, 0.0f, 0.0f, 1.0f}};
    return {temp};
  }

  friend mat4 operator*(const mat4 &a, const mat4 &b) {
    mat4 retVal;
    // row 0
    retVal.mat[0][0] = a.mat[0][0] * b.mat[0][0] + a.mat[0][1] * b.mat[1][0] +
                       a.mat[0][2] * b.mat[2][0] + a.mat[0][3] * b.mat[3][0];

    retVal.mat[0][1] = a.mat[0][0] * b.mat[0][1] + a.mat[0][1] * b.mat[1][1] +
                       a.mat[0][2] * b.mat[2][1] + a.mat[0][3] * b.mat[3][1];

    retVal.mat[0][2] = a.mat[0][0] * b.mat[0][2] + a.mat[0][1] * b.mat[1][2] +
                       a.mat[0][2] * b.mat[2][2] + a.mat[0][3] * b.mat[3][2];

    retVal.mat[0][3] = a.mat[0][0] * b.mat[0][3] + a.mat[0][1] * b.mat[1][3] +
                       a.mat[0][2] * b.mat[2][3] + a.mat[0][3] * b.mat[3][3];

    // row 1
    retVal.mat[1][0] = a.mat[1][0] * b.mat[0][0] + a.mat[1][1] * b.mat[1][0] +
                       a.mat[1][2] * b.mat[2][0] + a.mat[1][3] * b.mat[3][0];

    retVal.mat[1][1] = a.mat[1][0] * b.mat[0][1] + a.mat[1][1] * b.mat[1][1] +
                       a.mat[1][2] * b.mat[2][1] + a.mat[1][3] * b.mat[3][1];

    retVal.mat[1][2] = a.mat[1][0] * b.mat[0][2] + a.mat[1][1] * b.mat[1][2] +
                       a.mat[1][2] * b.mat[2][2] + a.mat[1][3] * b.mat[3][2];

    retVal.mat[1][3] = a.mat[1][0] * b.mat[0][3] + a.mat[1][1] * b.mat[1][3] +
                       a.mat[1][2] * b.mat[2][3] + a.mat[1][3] * b.mat[3][3];

    // row 2
    retVal.mat[2][0] = a.mat[2][0] * b.mat[0][0] + a.mat[2][1] * b.mat[1][0] +
                       a.mat[2][2] * b.mat[2][0] + a.mat[2][3] * b.mat[3][0];

    retVal.mat[2][1] = a.mat[2][0] * b.mat[0][1] + a.mat[2][1] * b.mat[1][1] +
                       a.mat[2][2] * b.mat[2][1] + a.mat[2][3] * b.mat[3][1];

    retVal.mat[2][2] = a.mat[2][0] * b.mat[0][2] + a.mat[2][1] * b.mat[1][2] +
                       a.mat[2][2] * b.mat[2][2] + a.mat[2][3] * b.mat[3][2];

    retVal.mat[2][3] = a.mat[2][0] * b.mat[0][3] + a.mat[2][1] * b.mat[1][3] +
                       a.mat[2][2] * b.mat[2][3] + a.mat[2][3] * b.mat[3][3];

    // row 3
    retVal.mat[3][0] = a.mat[3][0] * b.mat[0][0] + a.mat[3][1] * b.mat[1][0] +
                       a.mat[3][2] * b.mat[2][0] + a.mat[3][3] * b.mat[3][0];

    retVal.mat[3][1] = a.mat[3][0] * b.mat[0][1] + a.mat[3][1] * b.mat[1][1] +
                       a.mat[3][2] * b.mat[2][1] + a.mat[3][3] * b.mat[3][1];

    retVal.mat[3][2] = a.mat[3][0] * b.mat[0][2] + a.mat[3][1] * b.mat[1][2] +
                       a.mat[3][2] * b.mat[2][2] + a.mat[3][3] * b.mat[3][2];

    retVal.mat[3][3] = a.mat[3][0] * b.mat[0][3] + a.mat[3][1] * b.mat[1][3] +
                       a.mat[3][2] * b.mat[2][3] + a.mat[3][3] * b.mat[3][3];

    return retVal;
  }

  void invert() {
    // from MESA, via
    // http://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix
    const f32 inv[16] = {
        cell[5] * cell[10] * cell[15] - cell[5] * cell[11] * cell[14] -
            cell[9] * cell[6] * cell[15] + cell[9] * cell[7] * cell[14] +
            cell[13] * cell[6] * cell[11] - cell[13] * cell[7] * cell[10],
        -cell[1] * cell[10] * cell[15] + cell[1] * cell[11] * cell[14] +
            cell[9] * cell[2] * cell[15] - cell[9] * cell[3] * cell[14] -
            cell[13] * cell[2] * cell[11] + cell[13] * cell[3] * cell[10],
        cell[1] * cell[6] * cell[15] - cell[1] * cell[7] * cell[14] -
            cell[5] * cell[2] * cell[15] + cell[5] * cell[3] * cell[14] +
            cell[13] * cell[2] * cell[7] - cell[13] * cell[3] * cell[6],
        -cell[1] * cell[6] * cell[11] + cell[1] * cell[7] * cell[10] +
            cell[5] * cell[2] * cell[11] - cell[5] * cell[3] * cell[10] -
            cell[9] * cell[2] * cell[7] + cell[9] * cell[3] * cell[6],
        -cell[4] * cell[10] * cell[15] + cell[4] * cell[11] * cell[14] +
            cell[8] * cell[6] * cell[15] - cell[8] * cell[7] * cell[14] -
            cell[12] * cell[6] * cell[11] + cell[12] * cell[7] * cell[10],
        cell[0] * cell[10] * cell[15] - cell[0] * cell[11] * cell[14] -
            cell[8] * cell[2] * cell[15] + cell[8] * cell[3] * cell[14] +
            cell[12] * cell[2] * cell[11] - cell[12] * cell[3] * cell[10],
        -cell[0] * cell[6] * cell[15] + cell[0] * cell[7] * cell[14] +
            cell[4] * cell[2] * cell[15] - cell[4] * cell[3] * cell[14] -
            cell[12] * cell[2] * cell[7] + cell[12] * cell[3] * cell[6],
        cell[0] * cell[6] * cell[11] - cell[0] * cell[7] * cell[10] -
            cell[4] * cell[2] * cell[11] + cell[4] * cell[3] * cell[10] +
            cell[8] * cell[2] * cell[7] - cell[8] * cell[3] * cell[6],
        cell[4] * cell[9] * cell[15] - cell[4] * cell[11] * cell[13] -
            cell[8] * cell[5] * cell[15] + cell[8] * cell[7] * cell[13] +
            cell[12] * cell[5] * cell[11] - cell[12] * cell[7] * cell[9],
        -cell[0] * cell[9] * cell[15] + cell[0] * cell[11] * cell[13] +
            cell[8] * cell[1] * cell[15] - cell[8] * cell[3] * cell[13] -
            cell[12] * cell[1] * cell[11] + cell[12] * cell[3] * cell[9],
        cell[0] * cell[5] * cell[15] - cell[0] * cell[7] * cell[13] -
            cell[4] * cell[1] * cell[15] + cell[4] * cell[3] * cell[13] +
            cell[12] * cell[1] * cell[7] - cell[12] * cell[3] * cell[5],
        -cell[0] * cell[5] * cell[11] + cell[0] * cell[7] * cell[9] +
            cell[4] * cell[1] * cell[11] - cell[4] * cell[3] * cell[9] -
            cell[8] * cell[1] * cell[7] + cell[8] * cell[3] * cell[5],
        -cell[4] * cell[9] * cell[14] + cell[4] * cell[10] * cell[13] +
            cell[8] * cell[5] * cell[14] - cell[8] * cell[6] * cell[13] -
            cell[12] * cell[5] * cell[10] + cell[12] * cell[6] * cell[9],
        cell[0] * cell[9] * cell[14] - cell[0] * cell[10] * cell[13] -
            cell[8] * cell[1] * cell[14] + cell[8] * cell[2] * cell[13] +
            cell[12] * cell[1] * cell[10] - cell[12] * cell[2] * cell[9],
        -cell[0] * cell[5] * cell[14] + cell[0] * cell[6] * cell[13] +
            cell[4] * cell[1] * cell[14] - cell[4] * cell[2] * cell[13] -
            cell[12] * cell[1] * cell[6] + cell[12] * cell[2] * cell[5],
        cell[0] * cell[5] * cell[10] - cell[0] * cell[6] * cell[9] -
            cell[4] * cell[1] * cell[10] + cell[4] * cell[2] * cell[9] +
            cell[8] * cell[1] * cell[6] - cell[8] * cell[2] * cell[5]};

    const f32 det = cell[0] * inv[0] + cell[1] * inv[4] + cell[2] * inv[8] +
                    cell[3] * inv[12];

    if (det != 0) {
      const f32 invdet = 1.0f / det;
      for (int i = 0; i < 16; i++)
        cell[i] = inv[i] * invdet;
    }
  }

  f32 *getTranslation() { return mat[3]; }
};

vec4 operator*(const vec4 &v, const mat4 &M);
} // namespace Maths
#endif
