// SOURCE: https://learnopengl.com/Advanced-OpenGL/Framebuffers
#version 330

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D blurTexture;

void main()
{
        FragColor = texture(screenTexture, TexCoords);
}
