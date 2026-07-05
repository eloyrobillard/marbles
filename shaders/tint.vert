#version 430

uniform mat4 uViewProj;
uniform mat4 uModel;

layout(location = 0) in vec3 inPosition;

void main()
{
        vec4 pos = vec4(inPosition, 1.0);

        gl_Position = pos * uModel * uViewProj;
}
