#version 330

in vec2 texCoords;

in vec4 fragPosLightSpace;
in vec3 fragWorldPos;
in vec3 fragNormal;

out vec4 outColor;

// Normalized
uniform vec3 lightDir;

uniform float near;
uniform float far;

uniform sampler2D meshTex;
uniform sampler2D depthTex;
uniform sampler2D shadowMap;

float getShadow() {
        vec3 mapCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
        mapCoords = mapCoords * 0.5 + 0.5;

        // Compute depth to check against value saved in shadow map
        // If the current depth is higher (fragment is farther) then it should receive a shadow
        float closestDepth = texture(shadowMap, mapCoords.xy).r;
        float currentDepth = mapCoords.z;

        // TODO: prevent shadow on objects beyond the 'far' side of the orthogonal view

        if (currentDepth > closestDepth) {
                return 1.0;
        }

        return 0.0;
}

void main() {
        float shadow = getShadow();

        vec4 color = texture(depthTex, texCoords);
        if (shadow > 0.0) {
                outColor = color + vec4(vec3(0.002), 1.0);
        } else {
                // Add shadow if fragment is facing away from light
                if (dot(lightDir, fragNormal) > 0.0) {
                        color += vec4(vec3(0.002), 0.0);
                }
                outColor = color;
        }
}
