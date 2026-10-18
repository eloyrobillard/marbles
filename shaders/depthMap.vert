#version 330

uniform mat4 uWorldTransform;
uniform mat4 uViewProj;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

out vec4 fragNormal;
out vec3 fragNormalized;

void main() {
  // 法線に平行移動が適用されないように、w=0 とする
  fragNormal = vec4(normalize(inNormal * mat3(transpose(inverse(uWorldTransform)))), 0.0f);

  vec4 pos = vec4(inPosition, 1.0);
  pos = pos * uWorldTransform;
  pos = pos * uViewProj;
  fragNormalized = (pos.xyz / pos.w + 1.f) / 2.f;
  gl_Position = pos;
}
