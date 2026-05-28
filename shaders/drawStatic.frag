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

        float fragDepth = mapCoords.z;

        // Prevent shadow on objects beyond the 'far' side of the orthogonal view
        if (fragDepth > 1.0) return 0.0;

        float shadow = 0.0;
        int shadowSamples = 2;
        vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
        for (int x = -shadowSamples; x <= shadowSamples; x++) {
                for (int y = -shadowSamples; y <= shadowSamples; y++) {
                        float pcfDepth = texture(shadowMap, mapCoords.xy + vec2(x, y) * texelSize).r;

                        if (fragDepth - 0.002 > pcfDepth) {
                                shadow += 1.0;
                        }
                }
        }

        return shadow / pow(shadowSamples + 1, 2);
}

void main() {
        float shadow = getShadow();

        vec4 color = texture(depthTex, texCoords);
        if (shadow > 0.0) {
                outColor = color + vec4(vec3(0.002), 1.0) * shadow;
        } else {
                // Add shadow if fragment is facing away from light
                if (dot(lightDir, fragNormal) > 0.0) {
                        color += vec4(vec3(0.002), 0.0);
                }
                outColor = color;
        }
}
