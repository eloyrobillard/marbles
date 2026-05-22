#version 330 core

out vec2 texCoords;

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;

void main()
{
        texCoords = aTexCoords;
        gl_Position = vec4(aPos, 1.0);
}
