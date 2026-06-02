#version 330

in vec2 texCoords;

in vec4 fragPosLightSpaceStatic;
in vec4 fragPosLightSpaceMarble;
in vec3 fragWorldPos;
in vec3 fragNormal;

out vec4 outColor;

uniform vec3 lightDir;

uniform float near;
uniform float far;

uniform sampler2D meshTex;
uniform sampler2D depthTex;
uniform sampler2D shadowMapStatic;
uniform sampler2D shadowMapMarble;

// Define Poisson disk sampling values
// SOURCE: https://sibras.github.io/OpenGL4-Tutorials/docs/Tutorials/07-Tutorial7/#part-4-percentage-closer-filtering
const vec2 v2PoissonDisk[9] = vec2[](
                vec2(-0.01529481f, -0.07395129f),
                vec2(-0.56232890f, -0.36484920f),
                vec2(0.95519960f, 0.18418130f),
                vec2(0.20716880f, 0.49262790f),
                vec2(-0.01290792f, -0.95755550f),
                vec2(0.68047200f, -0.51716110f),
                vec2(-0.60139470f, 0.37665210f),
                vec2(-0.40243310f, 0.86631060f),
                vec2(-0.96646290f, -0.04688413f));

float getShadow(vec4 lightSpace, sampler2D shadowMap) {
        vec3 mapCoords = lightSpace.xyz / lightSpace.w;
        mapCoords = mapCoords * 0.5 + 0.5;

        float fragDepth = mapCoords.z;

        // Prevent shadow on objects beyond the 'far' side of the orthogonal view
        if (fragDepth > 1.0) return 0.0;

        float shadow = 0.0;
        vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

        for (int i = 0; i < 9; i++) {
                vec2 offset = v2PoissonDisk[i];
                float pcfDepth = texture(shadowMap, mapCoords.xy + offset * texelSize).r;

                if (fragDepth - 0.001 > pcfDepth) {
                        shadow += 1.0;
                }
        }

        return shadow / 9.0;
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
