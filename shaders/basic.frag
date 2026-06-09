#version 430

// Tex coord input from vertex shader
in vec2 fragTexCoord;

// This corresponds to the output color to the color buffer
layout(location = 0) out vec4 outColor;
// Output to the bloom color buffer
layout(location = 1) out vec4 brightColor;

// This is used for the texture sampling
uniform sampler2D uSamplingTexture;

void main()
{
        vec3 color = texture(uSamplingTexture, fragTexCoord).rgb;
        outColor = vec4(color, 1.0);

        // Check if brightness is above bloom threshold
        float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
        if (brightness > 1)
                brightColor = vec4(color, 1.0);
        else
                brightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
