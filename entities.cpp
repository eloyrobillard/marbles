#include "entities.h"

Entities::Entities() {
  RegisterEntities(
      {{"assets/ramp1.gpmesh", BodyType::Static, 1.0f, false, vec3::zero},
       {"assets/ramp2.gpmesh", BodyType::Static, 1.0f, false, vec3::zero},
       {"assets/ramp3.gpmesh", BodyType::Static, 1.0f, false, vec3::zero},
       {"assets/snake1.gpmesh", BodyType::Static, 1.0f, false, vec3::zero},
       {"assets/plane1.gpmesh", BodyType::Static, 1.0f, false, vec3::zero},
       {"assets/plane2.gpmesh", BodyType::Static, 1.0f, false, vec3::zero},
       {"assets/plane3.gpmesh", BodyType::Static, 1.0f, false, vec3::zero},
       {"assets/plane4.gpmesh", BodyType::Static, 1.0f, false, vec3::zero},
       {"assets/plane5.gpmesh", BodyType::Static, 1.0f, false, vec3::zero},
       {"assets/plane6.gpmesh", BodyType::Static, 1.0f, false, vec3::zero},
       {"assets/plane7.gpmesh", BodyType::Static, 1.0f, false, vec3::zero},
       {"assets/plane8.gpmesh", BodyType::Static, 1.0f, false, vec3::zero},
       {"assets/canon1.gpmesh", BodyType::Static, 1.03f, true, vec3::up},
       {"assets/sphere.gpmesh", BodyType::Dynamic, 1.0f, false, vec3::zero}});
}

void Entities::Update(float time, float deltaTime) {
  for (auto &dynamicEntity : mDynamicEntities) {
    dynamicEntity.Update(time, deltaTime, gSP);
  }
}

void DynamicEntity::Update(float t, float dt, const SpacePartition &sp) {
  const vec3 prev_p = body.position;

  body.velocity += dt * grav_force;
  body.position += dt * body.velocity;

  // Test collisions at new body position
  collider.position = body.position;

  // Instantly apply collisions to the velocity of the body
  Physics::getCollisionImpulse(sp, collider, body.velocity);

  // Adjust position based on (possibly) updated velocity
  body.position = prev_p + dt * body.velocity;

  // Match body's position with collider's
  collider.position = body.position;
}

void Entity::Draw(Shader &shader) const {
  // Set world transform
  mat4 worldTransform = body.getWorldTransform();

  shader.setMatrixUniform("uWorldTransform", worldTransform);

  auto maybe_tex = mesh.lookTextureUp(0);
  if (maybe_tex.has_value())
    Texture::SetActive(maybe_tex.value()->textureID);

  Shader::setVerticesActive(mesh.GetVertexArray());

  // Draw triangles
  glDrawElements(GL_TRIANGLES, static_cast<int>(mesh.GetNumIndices()),
                 GL_UNSIGNED_INT, nullptr);

  GLenum err_code = glGetError();
  while (GL_NO_ERROR != err_code) {
    printf("OpenGL Error @ %s: %i", "mesh draw", err_code);
    err_code = glGetError();
  }
}

void Entities::RegisterEntities(
    const vector<tuple<string, BodyType, float, bool, vec3>> &entityList) {
  for (const auto &[meshName, btype, accel, override_impulse,
                    impulse_override] : entityList) {
    auto maybe = Mesh::Load(meshName);

    if (maybe.has_value()) {
      auto [mesh, body] = maybe.value();

      if (btype == BodyType::Dynamic) {
        DynamicEntity de(mesh, body,
                         SphereCollider(body.position, body.scale.x));
        mDynamicEntities.emplace_back(de);
        mDynamicEntitiesStartingState.emplace_back(de);
      } else {
        auto triangles = mesh.generateTriangleCollidersFromMesh(
            body, accel, override_impulse, impulse_override);
        gSP.populate(triangles);

        mStaticEntities.emplace_back(mesh, body);
        mStaticColliders.emplace_back(triangles);
      }
    }
  }
}

void Entities::RegisterInputLeft(float dt) {
  mDynamicEntities[0].RegisterInputLeft(dt);
}

void Entities::RegisterInputRight(float dt) {
  mDynamicEntities[0].RegisterInputRight(dt);
}

void Entities::ToCheckpoint(const vector<vec3> &positionsAtCheckpoint) {
  for (int i = 0; i < positionsAtCheckpoint.size(); i++) {
    mDynamicEntities[i].ResetToPosition(positionsAtCheckpoint[i]);
  }
}
