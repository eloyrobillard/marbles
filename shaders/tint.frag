#version 430

out vec4 outColor;
uniform vec3 tint;

void main()
{
        outColor = vec4(tint, 1.0f);
};
