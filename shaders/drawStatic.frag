#version 330

in vec2 texCoords;

out vec4 outColor;

uniform sampler2D meshTex;
uniform sampler2D depthTex;

void main() {
        outColor = texture(depthTex, texCoords);
}
