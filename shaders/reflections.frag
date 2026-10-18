#version 330

in vec2 texCoords;
in vec3 fragNormal;
in vec3 fragWorldPos;

out vec4 outColor;

uniform sampler2D depthTex;
uniform sampler2D normalTex;
uniform sampler2D colorTex;
uniform vec3 cameraPos;

uniform mat4 uViewProj;

void main() {
  vec4 depth = texture(depthTex, texCoords);
  vec4 normal = texture(normalTex, texCoords);
  vec4 color = texture(colorTex, texCoords);

  // 最初からメッシュの後ろにいたらスキップ
  // vec3 v1 = fragWorldPos - cameraPos;
  // vec3 v2 = pos.xyz - cameraPos;
  // if (dot(v1, v1) - dot(v2, v2) > 1.f) {
  //   outColor = color;
  //   return;
  // }

  // NOTE: 今のところ、黒色の場合のみ反射を描画
  if (dot(color.xyz, vec3(1)) != 0) {
    outColor = color;
    return;
  }

  // Camera to fragment position
  vec3 pos = normalize(vec3(texCoords.xy, depth));
  vec3 toPos = pos - cameraPos;
  if (dot(toPos, toPos) > 100.f) {
    outColor = color;
    return;
  }

  vec4 pos4 = vec4(pos.xyz, 1.f);

  toPos = normalize(toPos);

  vec4 reflectedDir = vec4(reflect(toPos, normalize(normal.xyz)), 0);

  float step = 0.15f;
  float maxDist = 10.f;

  for (float d = 0.01f; d <= maxDist; d += step) {
    // ray march
    vec4 curPos = (pos4 + d * reflectedDir);
    vec4 curPosProj = curPos * uViewProj;
    vec2 uv = curPosProj.xy / (curPosProj.w * 2.f) + 0.5f;

    if (uv.x > 1 || uv.y > 1 || uv.x < 0 || uv.y < 0)
      break;

    float depthAtUV = texture(depthTex, uv).r;

    if (curPosProj.z > depthAtUV) {
      // if (0.000001f < res && res <= 0.5f) {
      // gone through something!
      // FIX: 衝突判定の精度を上げよ！

      outColor = texture(colorTex, uv);
      break;
      // } else if (res > 0.5f) {
      //   outColor = color;
      //   break;
    }
  }
}
