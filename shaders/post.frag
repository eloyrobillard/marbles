// version 330 core
// SOURCE: https://learnopengl.com/Advanced-OpenGL/Framebuffers
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
        FragColor = texture(screenTexture, TexCoords);
}
