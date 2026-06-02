#version 330

out vec2 texCoords;

out vec4 fragPosLightSpaceStatic;
out vec4 fragPosLightSpaceMarble;
out vec3 fragWorldPos;
out vec3 fragNormal;

uniform mat4 uWorldTransform;
uniform mat4 viewProj;

uniform mat4 lightViewProjMarble;
uniform mat4 lightViewProjStatic;
uniform vec3 lightDir;

// Attribute 0 is position, 1 is normal, 2 is tex coords.
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

void main()
{
        vec4 pos = vec4(inPosition, 1.0);
        pos = pos * uWorldTransform;

        // Send world position to fragment shader
        fragWorldPos = pos.xyz;
        fragPosLightSpaceStatic = pos * lightViewProjStatic;
        fragPosLightSpaceMarble = pos * lightViewProjMarble;

        fragNormal = (vec4(inNormal, 0.0f) * transpose(inverse(uWorldTransform))).xyz;

        pos = pos * viewProj;

        // Normalize x, y coordinates to screen range (0 to 1)
        texCoords = (pos.xy / pos.w + vec2(1.0)) / 2;

        gl_Position = pos;
}
