#include "Plane.hpp"

namespace GLOO {
Plane::Plane(const glm::vec3& normal, float d) {
  normal_ = normal;
  d_ = d;
}

bool Plane::Intersect(const Ray& ray, float t_min, HitRecord& record) const {
  // TODO: Implement ray-plane intersection.
  if (glm::dot(ray.GetDirection(), normal_) == 0) {
    //parallel to the plane
    return false;
  }
  float t = (d_ - glm::dot(ray.GetOrigin(), normal_)) / glm::dot(ray.GetDirection(), normal_);
  if (t < t_min) {
    return false;
  }
  record.time = t;
  record.normal = normal_;
  return true;
}
}  // namespace GLOO
