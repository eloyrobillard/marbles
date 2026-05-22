#version 330

in vec2 texCoords;

out vec4 outColor;

uniform sampler2D depthTex;

void main() {
        float current = texture(depthTex, texCoords).r;
        vec2 texSize = textureSize(depthTex, 0);
        vec2 offset = 1 / texSize;
        float threshold = 0.001;

        // if any of neighboring texels has a vastly different value
        // we're on the contour
        for (int x = 1; x < 3; x++) {
                for (int y = 1; y < 3; y++) {
                        vec2 coord1 = texCoords + offset * vec2(x, y);
                        float val1 = texture(depthTex, coord1).r;
                        if (abs(val1 - current) > threshold) {
                                outColor = vec4(1.0);
                                return;
                        }
                        vec2 coord2 = texCoords + offset * vec2(x, -y);
                        float val2 = texture(depthTex, coord2).r;
                        if (abs(val2 - current) > threshold) {
                                outColor = vec4(1.0);
                                return;
                        }
                        vec2 coord3 = texCoords + offset * vec2(-x, y);
                        float val3 = texture(depthTex, coord3).r;
                        if (abs(val3 - current) > threshold) {
                                outColor = vec4(1.0);
                                return;
                        }
                        vec2 coord4 = texCoords + offset * vec2(-x, -y);
                        float val4 = texture(depthTex, coord4).r;
                        if (abs(val4 - current) > threshold) {
                                outColor = vec4(1.0);
                                return;
                        }
                }
        }

        outColor = vec4(0.0);
}
