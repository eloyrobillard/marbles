#include "maths.hpp"

namespace Maths {
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
} // namespace Maths
