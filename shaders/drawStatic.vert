#version 330 core

out vec2 texCoords;

uniform mat4 uWorldTransform;
uniform mat4 uViewProj;

// Attribute 0 is position, 1 is normal, 2 is tex coords.
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

void main()
{
        vec4 pos = vec4(inPosition, 1.0);
        pos = pos * uWorldTransform;
        pos = pos * uViewProj;

        // Normalize x, y coordinates to screen range (0 to 1)
        texCoords = (pos.xy / pos.w + vec2(1.0)) / 2;

        gl_Position = pos;
}
