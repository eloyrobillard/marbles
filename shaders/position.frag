#version 330

in vec3 fragWorldPos;

out vec3 color;

void main() {
  color = fragWorldPos;
}
