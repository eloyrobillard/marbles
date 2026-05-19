#version 330 core

in vec2 texCoords;
out vec4 outColor;

uniform sampler2D normalTex;
uniform sampler2D bloomTex;

void main() {
        vec3 color = texture(normalTex, texCoords).rgb + texture(bloomTex, texCoords).rgb;
        outColor = vec4(color, 1.0);
}
