// SOURCE: https://learnopengl.com/code_viewer_gh.php?code=src/4.advanced_opengl/6.1.cubemaps_skybox/6.1.skybox.fs
#version 330
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
        FragColor = texture(skybox, TexCoords);
}
