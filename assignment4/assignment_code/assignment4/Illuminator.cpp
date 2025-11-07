#include "Illuminator.hpp"

#include <limits>
#include <stdexcept>

#include <glm/geometric.hpp>

#include "gloo/lights/DirectionalLight.hpp"
#include "gloo/lights/PointLight.hpp"
#include "gloo/SceneNode.hpp"

namespace GLOO {
void Illuminator::GetIllumination(const LightComponent& light_component,
                                  const glm::vec3& hit_pos,
                                  glm::vec3& dir_to_light,
                                  glm::vec3& intensity,
                                  float& dist_to_light) {
  // Calculation will be done in world space.

  auto light_ptr = light_component.GetLightPtr();
  if (light_ptr->GetType() == LightType::Directional) {
    auto directional_light_ptr = static_cast<DirectionalLight*>(light_ptr);
    dir_to_light = -directional_light_ptr->GetDirection();
    intensity = directional_light_ptr->GetDiffuseColor();
    dist_to_light = std::numeric_limits<float>::max();
  } else if (light_ptr->GetType() == LightType::Point) {
    auto point_light_ptr = static_cast<PointLight*>(light_ptr);
    
    // Get the light position from the scene node
    glm::vec3 light_pos = light_component.GetNodePtr()->GetTransform().GetPosition();
    // Calculate direction vector from hit point to light (normalized)
    dir_to_light = glm::normalize(light_pos - hit_pos);
    
    // Calculate distance between hit point and light
    dist_to_light = glm::length(light_pos - hit_pos);
    
    // Calculate illumination intensity using formula: L(x_surf) = I / (αd²)
    // where I is the light color, d is distance, and α is attenuation factor
    glm::vec3 attenuation = point_light_ptr->GetAttenuation();
    float alpha = attenuation.x;  // Use the constant attenuation factor
    
    // Apply inverse square law with attenuation: I / (α * d²)
    intensity = point_light_ptr->GetDiffuseColor() / (alpha * dist_to_light * dist_to_light);
  }
}
}  // namespace GLOO
