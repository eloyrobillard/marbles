#version 430

in vec2 texCoords;
out vec4 outColor;

uniform sampler2D normalTex;
uniform sampler2D bloomTex;
uniform float exposure;

// SOURCE: https://learnopengl.com/Advanced-Lighting/Bloom
void main() {
        const float gamma = 2.2;
        vec3 hdrColor = texture(normalTex, texCoords).rgb;
        vec3 bloomColor = texture(bloomTex, texCoords).rgb;
        hdrColor += bloomColor; // additive blending

        // tone mapping
        vec3 result = vec3(1.0) - exp(-hdrColor * exposure);

        // also gamma correct while we're at it
        result = pow(result, vec3(1.0 / gamma));
        outColor = vec4(result, 1.0);
}
