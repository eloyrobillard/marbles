// SOURCE: https://learnopengl.com/Advanced-OpenGL/Framebuffers
# version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D blurTexture;

void main()
{
        FragColor = texture(screenTexture, TexCoords);
}
