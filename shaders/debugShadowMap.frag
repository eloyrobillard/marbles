# version 430

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D shadowMap;

void main()
{
        float depth = texture(shadowMap, TexCoords).r;
        FragColor = vec4(vec3(depth), 1.0);
}
