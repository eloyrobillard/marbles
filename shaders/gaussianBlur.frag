#version 330 core

in vec2 texCoords;

out vec4 fragColor;

uniform bool horizontal;
uniform sampler2D brightTexture;

uniform float weights[5] = float[](0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

void main() {
        vec2 texSize = textureSize(brightTexture, 0);
        vec2 offset = 1.0 / texSize;

        vec3 res = texture(brightTexture, texCoords).rgb * weights[0];

        if (horizontal)
                for (int i = 1; i < 5; i++) {
                        float weight = weights[i];
                        res += texture(brightTexture, vec2(i * offset.x + texCoords.x, texCoords.y)).rgb * weight;
                        res += texture(brightTexture, vec2(-i * offset.x + texCoords.x, texCoords.y)).rgb * weight;
                }
        else
                for (int i = 1; i < 5; i++) {
                        float weight = weights[i];
                        res += texture(brightTexture, vec2(texCoords.x, i * offset.y + texCoords.y)).rgb * weight;
                        res += texture(brightTexture, vec2(texCoords.x, -i * offset.y + texCoords.y)).rgb * weight;
                }

        fragColor = vec4(res, 1.0);
}
