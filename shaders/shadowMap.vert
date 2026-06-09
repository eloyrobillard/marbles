#version 430

layout(location = 0) in vec3 inPosition;

out vec4 fragPos;

uniform mat4 uWorldTransform;
uniform mat4 uViewProj;

void main() {
        vec4 pos = vec4(inPosition, 1.0) * uWorldTransform;
        fragPos = pos * uViewProj;
        gl_Position = fragPos;
}
