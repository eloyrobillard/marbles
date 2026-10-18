#version 330

in vec4 fragNormal;
in vec3 fragNormalized;

layout(location = 0) out vec4 normal;
layout(location = 1) out vec4 albedo;

uniform sampler2D uSamplingTexture;

void main() {
  normal = fragNormal;
  albedo = texture(uSamplingTexture, fragNormalized.xy);
}
