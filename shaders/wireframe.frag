// Request GLSL 3.3
#version 430

// This corresponds to the output color to the color buffer
out vec4 outColor;
uniform vec3 tint;

void main()
{
        outColor = vec4(tint, 1.0f);
};
