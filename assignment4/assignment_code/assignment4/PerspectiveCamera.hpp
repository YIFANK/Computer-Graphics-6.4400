#ifndef PERSPECTIVE_CAMERA_H_
#define PERSPECTIVE_CAMERA_H_

#include <cmath>
#include <glm/ext/quaternion_geometric.hpp>

#include "gloo/utils.hpp"

#include "Ray.hpp"
#include "CameraSpec.hpp"
#include "CameraBase.hpp"
namespace GLOO {
class PerspectiveCamera : public CameraBase {
 public:
  PerspectiveCamera(const CameraSpec& spec) : CameraBase(spec) {}

  Ray GenerateRay(const glm::vec2& point) const override {
    float d = 1.0f / tanf(fov_radian_ / 2.0f);
    glm::vec3 new_dir =
        d * direction_ + point[0] * horizontal_ + point[1] * up_;
    new_dir = glm::normalize(new_dir);

    return Ray(center_, new_dir);
  }

  float GetTMin() const override {
    return 0.0f;
  }
};
}  // namespace GLOO

#endif
