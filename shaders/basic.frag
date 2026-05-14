#version 330

// Tex coord input from vertex shader
in vec2 fragTexCoord;

// This corresponds to the output color to the color buffer
out vec4 outColor;

// This is used for the texture sampling
uniform sampler2D uSamplingTexture;

void main()
{
        vec3 color = texture(uSamplingTexture, fragTexCoord).rgb;
        outColor = vec4(color, 1.0);
}
