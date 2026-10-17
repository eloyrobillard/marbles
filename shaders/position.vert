#version 330

uniform mat4 uWorldTransform;
uniform mat4 uViewProj;

out vec3 fragWorldPos;

layout(location = 0) in vec3 inPosition;

void main() {
  vec4 pos = vec4(inPosition, 1.0) * uWorldTransform;
  fragWorldPos = pos.xyz;
  gl_Position = pos * uViewProj;
}
