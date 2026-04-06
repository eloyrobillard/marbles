// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
//
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

// Request GLSL 3.3
#version 330

// Tex coord input from vertex shader
in vec2 fragTexCoord;

in vec3 fragNormal;
in vec3 fragWorldPos;

// This corresponds to the output color to the color buffer
out vec4 outColor;

// This is used for the texture sampling
uniform sampler2D uTexture;

struct DirectionalLight {
        vec3 direction;
        vec3 diffuseColor;
        vec3 specularColor;
};

uniform float uSpecPower;
uniform vec3 uCameraPos;
uniform vec3 uAmbientLight;
uniform DirectionalLight uDirLight;

void main()
{
        vec3 N = normalize(fragNormal);
        // Vector from surface to light
        vec3 L = normalize(-uDirLight.direction);
        // Vector from fragment to camera
        vec3 V = normalize(uCameraPos - fragWorldPos);
        vec3 R = normalize(reflect(-L, N));

        vec3 phong = uAmbientLight;
        float NdotL = dot(N, L);
        if (NdotL > 0) {
                vec3 diffuse = uDirLight.diffuseColor * NdotL;
                vec3 specular = uDirLight.specularColor * pow(max(0.0, dot(R, V)), uSpecPower);
                phong += diffuse + specular;
        }

        // Sample color from texture
        outColor = texture(uTexture, fragTexCoord) * vec4(phong, 1.0f);
};
