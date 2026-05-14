#version 330

// Tex coord input from vertex shader
in vec2 fragTexCoord;

in vec3 fragNormal;
in vec3 fragWorldPos;
in vec4 fragPosLightSpace;

// This corresponds to the output color to the color buffer
out vec4 outColor;

// This is used for the texture sampling
uniform sampler2D uSamplingTexture;

struct DirectionalLight {
        vec3 diffuseColor;
        vec3 specularColor;
};

uniform float uSpecPower;
uniform vec3 uCameraPos;
uniform vec3 uLightPos;
uniform vec3 uLightTarget;
uniform vec3 uAmbientLight;
uniform DirectionalLight uDirLight;

void main()
{
        vec3 color = texture(uSamplingTexture, fragTexCoord).rgb;
        vec3 normal = normalize(fragNormal);
        vec3 lightColor = vec3(0.3);

        // ambient
        vec3 ambient = uAmbientLight;

        // diffuse
        vec3 lightDir = normalize(uLightPos - uLightTarget);
        float diff = max(dot(lightDir, normal), 0.0);
        vec3 diffuse = diff * uDirLight.diffuseColor;

        // specular
        vec3 viewDir = normalize(uCameraPos - fragWorldPos);
        vec3 reflectDir = normalize(reflect(-lightDir, normal));
        float spec = pow(max(dot(normal, reflectDir), 0.0), 64.0);
        vec3 specular = spec * uDirLight.specularColor;

        vec3 lighting = (ambient + (diffuse + specular)) * color;

        outColor = vec4(lighting, 1.0);
}
