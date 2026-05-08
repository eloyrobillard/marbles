#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 uLightSpaceMatrix;
uniform mat4 uWorldTransform;

void main()
{
        vec4 pos = vec4(aPos, 1.0);
        pos = pos * uWorldTransform;
        gl_Position = pos * uLightSpaceMatrix;
}

// void main()
// {
//         gl_Position = uLightSpaceMatrix * uWorldTransform * vec4(aPos, 1.0);
// }
