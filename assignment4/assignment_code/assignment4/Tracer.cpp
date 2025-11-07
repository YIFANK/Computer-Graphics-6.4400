#include "Tracer.hpp"

#include <glm/gtx/string_cast.hpp>
#include <stdexcept>
#include <algorithm>

#include "gloo/Transform.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/lights/AmbientLight.hpp"

#include "gloo/Image.hpp"
#include "Illuminator.hpp"

namespace GLOO {
void Tracer::Render(const Scene& scene, const std::string& output_file) {
  scene_ptr_ = &scene;

  auto& root = scene_ptr_->GetRootNode();
  tracing_components_ = root.GetComponentPtrsInChildren<TracingComponent>();
  light_components_ = root.GetComponentPtrsInChildren<LightComponent>();


  Image image(image_size_.x, image_size_.y);
  int total_pixels = image_size_.x * image_size_.y;
  int current_pixel = 0,progress = 0;

  auto randf = []() {
    return static_cast<float>(rand()) / RAND_MAX - 0.5f; // in [-0.5,0.5)
  };

  auto sample_color = [&](size_t x, size_t y,bool randomize = true) {
    float u = x / (image_size_.x - 1);
    float v = y / (image_size_.y - 1);
    if (randomize) {
      u = (x + randf()) / (image_size_.x - 1);
      v = (y + randf()) / (image_size_.y - 1);
    }
    //make range to [-1,1]
    u = 2.0f * u - 1.0f;
    v = 2.0f * v - 1.0f;
    Ray ray = camera_->GenerateRay(glm::vec2(u, v));
    HitRecord record;
    record.time = std::numeric_limits<float>::max();
    return TraceRay(ray, max_bounces_, record);
  };
  for (size_t y = 0; y < image_size_.y; y++) {
    for (size_t x = 0; x < image_size_.x; x++) {
      glm::vec3 color(0.0f);
      if(samples_ == 1) {
        color = sample_color(x, y);
      } else {
        for (int s = 0; s < samples_; s++) {
          color += sample_color(x, y);
        }
        color /= samples_;
      }
      // Set the pixel color in the image
      image.SetPixel(x, y, color);
      current_pixel++;
    }
    float fprogress = 100.0f * current_pixel / total_pixels;
    if (fprogress > progress + 1) {
      progress = fprogress;
      std::cout << "Rendered: " << progress << "%" << std::endl;
    }
  }
  if (output_file.size())
    image.SavePNG(output_file);
}

bool Tracer::InShadow(const Ray& ray, float max_t) const {
  for (auto& component : tracing_components_) {
    const HittableBase& hittable = component->GetHittable();
    glm::mat4 local_to_world = component->GetNodePtr()->GetTransform().GetLocalToWorldMatrix();
    glm::mat4 world_to_local = glm::inverse(local_to_world);
    Ray local_ray(
      glm::vec3(world_to_local * glm::vec4(ray.GetOrigin(), 1.0f)),
      glm::vec3(world_to_local * glm::vec4(ray.GetDirection(), 0.0f))
    );
    HitRecord temp_record;
    temp_record.time = std::numeric_limits<float>::max();
    if (hittable.Intersect(local_ray, 0.001f, temp_record) && temp_record.time < max_t) {
      return true;
    }
  }
  return false;
}
glm::vec3 Tracer::TraceRay(const Ray& ray,
                           size_t bounces,
                           HitRecord& record) const {
  // TODO: Compute the color for the cast ray.
  bool hit_anything = false;
  const TracingComponent* hit_object = nullptr;
  auto clamp = [&](glm::vec3 A,glm::vec3 B) {
    return glm::max(0.0f,glm::dot(A,B));
  };
  for (auto& component : tracing_components_){
    const HittableBase& hittable = component->GetHittable();
    glm::mat4 local_to_world = component->GetNodePtr()->GetTransform().GetLocalToWorldMatrix();
    glm::mat4 world_to_local = glm::inverse(local_to_world);
    Ray local_ray(
      glm::vec3(world_to_local * glm::vec4(ray.GetOrigin(), 1.0f)),
      glm::vec3(world_to_local * glm::vec4(ray.GetDirection(), 0.0f))
    );
    HitRecord temp_record;
    temp_record.time = std::numeric_limits<float>::max();
    
    if (hittable.Intersect(local_ray, 0.001f, temp_record) && temp_record.time < record.time) {
      hit_object = component;
      hit_anything = true;
      record = temp_record;
    }
}

  if (hit_anything) {
    // Get the material component from the hit object's node
    auto material_component = hit_object->GetNodePtr()->GetComponentPtr<MaterialComponent>();
    
    if (material_component == nullptr) {
      return glm::vec3(1.0f, 0.0f, 1.0f); // Magenta for missing material
    }
    
    const auto& material = material_component->GetMaterial();
    glm::vec3 final_color(0.0f);
    glm::vec3 hit_pos = ray.At(record.time);
    // Get material properties
    glm::vec3 k_ambient = material.GetAmbientColor();
    glm::vec3 k_diffuse = material.GetDiffuseColor();
    glm::vec3 k_specular = material.GetSpecularColor();
    float shininess = material.GetShininess();
    for (auto& light : light_components_) {
      glm::vec3 light_intensity;
      glm::vec3 dir_to_light;
      float dist_to_light;
      Illuminator::GetIllumination(*light, hit_pos, dir_to_light, light_intensity, dist_to_light);
      //check if the light is in the shadow
      if (shadows_enabled_) {
        Ray shadow_ray(hit_pos, dir_to_light);
        float max_t = dist_to_light / glm::length(dir_to_light);
        if (InShadow(shadow_ray, max_t)) {
          continue;
        }
      }
      //check if the light is ambient
      if (light->GetLightPtr()->GetType() == LightType::Ambient) {
        glm::vec3 ambient = k_ambient * light->GetLightPtr()->GetDiffuseColor();
        final_color += ambient;
        continue;
      }
      //calculate diffuse light
      glm::vec3 diffuse = k_diffuse * light_intensity * clamp(record.normal, dir_to_light);
      //calculate specular light
      //find perfect reflection direction
      glm::vec3 R = glm::reflect(-dir_to_light, record.normal);
      glm::vec3 V = -ray.GetDirection();
      glm::vec3 specular = k_specular * light_intensity * glm::pow(clamp(R, V), shininess);
      //calculate ambient light
      final_color += diffuse + specular;
    }
    //add support for bounces
    if (bounces > 0) {
      Ray bounce_ray(hit_pos, glm::reflect(ray.GetDirection(), record.normal));
      HitRecord bounce_record;
      bounce_record.time = std::numeric_limits<float>::max();
      glm::vec3 bounce_color = TraceRay(bounce_ray, bounces - 1, bounce_record);
      final_color += bounce_color * k_specular;
    }
    return final_color;
  }
  return GetBackgroundColor(ray.GetDirection());
}

glm::vec3 Tracer::GetBackgroundColor(const glm::vec3& direction) const {
  if (cube_map_ != nullptr) {
    return cube_map_->GetTexel(direction);
  } else
    return background_color_;
}
}  // namespace GLOO
