// SOURCE: https://learnopengl.com/In-Practice/Text-Rendering
#version 430
layout(location = 0) in vec4 vertex;
out vec2 TexCoords;

void main()
{
        gl_Position = vec4(vertex.xy, 1.0, 1.0);
        TexCoords = (vertex.xy + vec2(1.0, 1.0)) / 2.0;
}
