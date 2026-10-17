#version 330

out vec2 texCoords;
out vec3 fragNormal;
out vec3 fragWorldPos;

uniform mat4 uWorldTransform;
uniform mat4 uViewProj;

// Attribute 0 is position, 1 is normal, 2 is tex coords.
layout(location = 0) in vec3 inPosition;
layout(location = 0) in vec3 inNormal;

void main()
{
  vec4 pos = vec4(inPosition, 1.0);
  pos = pos * uWorldTransform;
  fragWorldPos = pos.xyz;

  pos = pos * uViewProj;

  fragNormal = normalize(inNormal * mat3(transpose(inverse(uWorldTransform))));

  // Normalize x, y coordinates to screen range (0 to 1)
  texCoords = (pos.xy / pos.w) / 2.0f + 0.5f;

  gl_Position = pos;
}
