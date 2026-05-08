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
uniform sampler2D uDepthMap;

struct DirectionalLight {
        vec3 direction;
        vec3 diffuseColor;
        vec3 specularColor;
};

uniform float uSpecPower;
uniform vec3 uCameraPos;
uniform vec3 uLightPos;
uniform vec3 uAmbientLight;
uniform DirectionalLight uDirLight;

// SOURCE: https://learnopengl.com/code_viewer_gh.php?code=src/5.advanced_lighting/3.1.3.shadow_mapping/3.1.3.shadow_mapping.fs
float ShadowCalculation(vec4 fragPosLightSpace)
{
        // perform perspective divide
        vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
        // transform to [0,1] range
        projCoords = projCoords * 0.5 + 0.5;
        // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
        float closestDepth = texture(uDepthMap, projCoords.xy).r;
        // get depth of current fragment from light's perspective
        float currentDepth = projCoords.z;
        // calculate bias (based on depth map resolution and slope)
        vec3 normal = normalize(fragNormal);
        vec3 lightDir = normalize(uLightPos - fragWorldPos);
        float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

        // check whether current frag pos is in shadow
        // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
        // PCF
        float shadow = 0.0;
        vec2 texelSize = 1.0 / textureSize(uDepthMap, 0);
        for (int x = -1; x <= 1; ++x)
        {
                for (int y = -1; y <= 1; ++y)
                {
                        float pcfDepth = texture(uDepthMap, projCoords.xy + vec2(x, y) * texelSize).r;
                        shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
                }
        }

        shadow /= 9.0;

        // keep the shadow at 0.0 when outside the far plane region of the light's frustum.
        if (projCoords.z > 1.0)
                shadow = 0.0;

        return shadow;
}

void main()
{
        vec3 color = texture(uSamplingTexture, fragTexCoord).rgb;
        vec3 normal = normalize(fragNormal);
        vec3 lightColor = vec3(0.3);

        // ambient
        vec3 ambient = uAmbientLight;

        // diffuse
        vec3 lightDir = -normalize(uDirLight.direction);
        float diff = max(dot(lightDir, normal), 0.0);
        vec3 diffuse = diff * uDirLight.diffuseColor;

        // specular
        vec3 viewDir = normalize(uCameraPos - fragWorldPos);
        vec3 reflectDir = normalize(reflect(-lightDir, normal));
        float spec = pow(max(dot(normal, reflectDir), 0.0), 64.0);
        vec3 specular = spec * uDirLight.specularColor;

        float shadow = ShadowCalculation(fragPosLightSpace);
        vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

        outColor = vec4(lighting, 1.0);
}
