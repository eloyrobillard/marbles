// SOURCE: https://learnopengl.com/In-Practice/Text-Rendering
#version 430
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform sampler2D screen;
uniform vec3 textColor;

void main()
{
        // vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
        // color = vec4(textColor, 1.0) * sampled;

        vec3 screenSample = texture(screen, TexCoords).rgb;

        // Increase readability by forcing color to black if the background is close to white
        if ((screenSample.r + screenSample.g + screenSample.b) / 3. > 0.9)
                color = vec4(vec3(0.0), texture(text, TexCoords).r);
        else
                color = vec4(vec3(1.0), texture(text, TexCoords).r);
}
