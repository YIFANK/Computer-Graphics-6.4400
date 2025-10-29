#include "Triangle.hpp"

#include <iostream>
#include <stdexcept>

#include <glm/common.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Plane.hpp"

namespace GLOO {
Triangle::Triangle(const glm::vec3& p0,
                   const glm::vec3& p1,
                   const glm::vec3& p2,
                   const glm::vec3& n0,
                   const glm::vec3& n1,
                   const glm::vec3& n2) {
  positions_ = {p0, p1, p2};
  normals_ = {n0, n1, n2};
}

Triangle::Triangle(const std::vector<glm::vec3>& positions,
                   const std::vector<glm::vec3>& normals) {
  positions_ = positions;
  normals_ = normals;
}

bool Triangle::Intersect(const Ray& ray, float t_min, HitRecord& record) const {
  // TODO: Implement ray-triangle intersection.
  glm::vec3 e1 = positions_[1] - positions_[0];
  glm::vec3 e2 = positions_[2] - positions_[0];
  glm::vec3 s1 = glm::cross(ray.GetDirection(), e2);
  float det = glm::dot(e1, s1);
  //on the same plane
  if (det < 1e-8f) {
    return false;
  }
  float inv_det = 1.0f / det;
  //calculate u and v
  glm::vec3 s = ray.GetOrigin() - positions_[0];
  float u = inv_det * glm::dot(s, s1);
  if (u < 0.0f || u > 1.0f) {
    return false;
  }
  // Calculate barycentric coordinate v
  glm::vec3 s2 = glm::cross(s, e1);
  float v = inv_det * glm::dot(ray.GetDirection(), s2);
  
  // Check if intersection is outside triangle (v < 0 or u + v > 1)
  if (v < 0.0f || u + v > 1.0f) {
    return false;
  }
  //calculate t
  float t = inv_det * glm::dot(e2, s2);
  if (t < t_min) {
    return false;
  }
  record.time = t;
  //interpolate normal
  record.normal = glm::normalize(normals_[0] * (1.0f - u - v) + normals_[1] * u + normals_[2] * v);
  return true;
}
}  // namespace GLOO
