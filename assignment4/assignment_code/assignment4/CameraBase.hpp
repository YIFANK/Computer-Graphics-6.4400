#ifndef CAMERA_BASE_H_
#define CAMERA_BASE_H_

#include "CameraSpec.hpp"
#include "Ray.hpp"
#include <glm/glm.hpp>

namespace GLOO {
class CameraBase {
 public:
  CameraBase(const CameraSpec& spec)
      : center_(spec.center),
        direction_(glm::normalize(spec.direction)),
        up_(glm::normalize(spec.up)),
        fov_radian_(ToRadian(spec.fov)),
        horizontal_(glm::normalize(glm::cross(direction_, up_))) {}

  virtual ~CameraBase() {}

  virtual Ray GenerateRay(const glm::vec2& point) const = 0;

  virtual float GetTMin() const = 0;

 protected:
  glm::vec3 center_;
  glm::vec3 direction_;
  glm::vec3 up_;
  float fov_radian_;
  glm::vec3 horizontal_;
};
}  // namespace GLOO
#endif
