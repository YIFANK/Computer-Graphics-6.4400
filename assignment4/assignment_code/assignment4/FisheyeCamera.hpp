#ifndef FISHEYE_CAMERA_H_
#define FISHEYE_CAMERA_H_

#include <cmath>
#include <glm/ext/quaternion_geometric.hpp>

#include "gloo/utils.hpp"

#include "Ray.hpp"
#include "CameraSpec.hpp"
#include "CameraBase.hpp"
namespace GLOO {
class FisheyeCamera : public CameraBase {
 public:
  FisheyeCamera(const CameraSpec& spec, float fisheye_strength = 3.0f)
      : CameraBase(spec), fisheye_strength_(fisheye_strength) {
  }

  Ray GenerateRay(const glm::vec2& point) const override {
    // Calculate distance from center of image plane
    float r = glm::length(point);
    
    // Apply fisheye distortion
    // For r=0 (center), no distortion
    // For larger r, apply increasing angular distortion
    float theta = r * fov_radian_ * fisheye_strength_;
    
    // Avoid division by zero at center
    float phi = (r > 1e-6f) ? atan2f(point[1], point[0]) : 0.0f;
    
    // Convert back to direction using spherical coordinates
    float sin_theta = sinf(theta);
    float cos_theta = cosf(theta);
    
    // Create ray direction in camera space
    glm::vec3 new_dir = cos_theta * direction_ + 
                        sin_theta * (cosf(phi) * horizontal_ + sinf(phi) * up_);
    
    new_dir = glm::normalize(new_dir);

    return Ray(center_, new_dir);
  }

  float GetTMin() const override {
    return 0.0f;
  }

 private:
  float fisheye_strength_;
};
}  // namespace GLOO

#endif