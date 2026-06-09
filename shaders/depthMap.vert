#version 430

uniform mat4 uWorldTransform;
uniform mat4 uViewProj;

layout(location = 0) in vec3 inPosition;

void main() {
        vec4 pos = vec4(inPosition, 1.0);
        pos = pos * uWorldTransform;
        gl_Position = pos * uViewProj;
}
