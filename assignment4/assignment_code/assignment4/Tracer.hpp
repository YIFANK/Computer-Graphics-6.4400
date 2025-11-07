#ifndef TRACER_H_
#define TRACER_H_

#include "gloo/Scene.hpp"
#include "gloo/Material.hpp"
#include "gloo/lights/LightBase.hpp"
#include "gloo/components/LightComponent.hpp"

#include "Ray.hpp"
#include "HitRecord.hpp"
#include "TracingComponent.hpp"
#include "CubeMap.hpp"
#include "PerspectiveCamera.hpp"
#include "FisheyeCamera.hpp"
#include "CameraBase.hpp"
#include "CameraType.hpp"
namespace GLOO {
class Tracer {
 public:
  Tracer(const CameraSpec& camera_spec,
         const glm::ivec2& image_size,
         size_t max_bounces,
         const glm::vec3& background_color,
         const CubeMap* cube_map,
         bool shadows_enabled,
         size_t samples,
         CameraType camera_type = CameraType::Perspective)
      : image_size_(image_size),
        max_bounces_(max_bounces),
        background_color_(background_color),
        cube_map_(cube_map),
        shadows_enabled_(shadows_enabled),
        samples_(samples),
        scene_ptr_(nullptr) {
          if (camera_type == CameraType::Perspective) {
            camera_ = make_unique<PerspectiveCamera>(camera_spec);
          } else if (camera_type == CameraType::Fisheye) {
            camera_ = make_unique<FisheyeCamera>(camera_spec);
          } else {
            throw std::invalid_argument("Invalid camera type");
          }
  }
  void Render(const Scene& scene, const std::string& output_file);

 private:
  glm::vec3 TraceRay(const Ray& ray, size_t bounces, HitRecord& record) const;
  bool InShadow(const Ray& ray, float max_t) const;
  glm::vec3 GetBackgroundColor(const glm::vec3& direction) const;

  std::unique_ptr<CameraBase> camera_;
  glm::ivec2 image_size_;
  size_t max_bounces_;

  std::vector<TracingComponent*> tracing_components_;
  std::vector<LightComponent*> light_components_;
  glm::vec3 background_color_;
  const CubeMap* cube_map_;
  bool shadows_enabled_;
  size_t samples_;
  const Scene* scene_ptr_;
};
}  // namespace GLOO

#endif
