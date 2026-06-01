#version 330

in vec2 texCoords;

in vec4 fragPosLightSpaceStatic;
in vec4 fragPosLightSpaceMarble;
in vec3 fragWorldPos;
in vec3 fragNormal;

out vec4 outColor;

// Normalized
uniform vec3 lightDir;

uniform float near;
uniform float far;

uniform sampler2D meshTex;
uniform sampler2D depthTex;
uniform sampler2D shadowMapStatic;
uniform sampler2D shadowMapMarble;

float samples[32] = {
                -1.5,
                1.5,
                -1.5,
                -0.5,
                0.5,
                1.5,
                0.5,
                -0.5,
                -1.5,
                0.5,
                -1.5,
                -1.5,
                0.5,
                0.5,
                0.5,
                -1.5,
                -0.5,
                1.5,
                -0.5,
                -0.5,
                1.5,
                1.5,
                1.5,
                -0.5,
                -0.5,
                0.5,
                -0.5,
                1.5,
                1.5,
                0.5,
                1.5,
                -1.5
        };

float getShadow(vec4 lightSpace, sampler2D shadowMap) {
        vec3 mapCoords = lightSpace.xyz / lightSpace.w;
        mapCoords = mapCoords * 0.5 + 0.5;

        float fragDepth = mapCoords.z;

        // Prevent shadow on objects beyond the 'far' side of the orthogonal view
        if (fragDepth > 1.0) return 0.0;

        float shadow = 0.0;
        vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
        ivec2 pattern = ivec2(lightSpace.xy);
        pattern = ivec2(mod(pattern, ivec2(2)));

        int si = pattern.x + pattern.y * 2;
        for (int i = 0; i < 4; i++) {
                float x = samples[si + i * 2];
                float y = samples[si + i * 2 + 1];
                float pcfDepth = texture(shadowMap, mapCoords.xy + vec2(x, y) * texelSize).r;

                if (fragDepth - 0.001 > pcfDepth) {
                        shadow += 1.0;
                }
        }

        return shadow / 4.0;
}

void main() {
        float staticShadow = getShadow(fragPosLightSpaceStatic, shadowMapStatic);
        float marbleShadow = getShadow(fragPosLightSpaceMarble, shadowMapMarble);

        vec4 color = texture(depthTex, texCoords);
        if (staticShadow > 0.0) {
                outColor = color + vec4(vec3(0.002), staticShadow);
        } else if (marbleShadow > 0.0) {
                outColor = color + vec4(vec3(0.002), marbleShadow);
        } else {
                // Add shadow if fragment is facing away from light
                if (dot(lightDir, fragNormal) > 0.0) {
                        color += vec4(vec3(0.002), 0.0);
                }

                outColor = color;
        }
}
