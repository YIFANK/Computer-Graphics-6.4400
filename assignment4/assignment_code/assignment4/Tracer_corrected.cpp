glm::vec3 Tracer::TraceRay(const Ray& ray,
                           size_t bounces,
                           HitRecord& record) const {
  // TODO: Compute the color for the cast ray.
  bool hit_anything = false;
  const TracingComponent* hit_object = nullptr;
  for (auto& component : tracing_components_){
    const HittableBase& hittable = component->GetHittable();
    glm::mat4 world_to_local = glm::inverse(component->GetNodePtr()->GetTransform().GetLocalToWorldMatrix());
    Ray local_ray(
      glm::vec3(world_to_local * glm::vec4(ray.GetOrigin(), 1.0f)),
      glm::vec3(world_to_local * glm::vec4(ray.GetDirection(), 0.0f))
    );
    HitRecord temp_record;
    temp_record.time = std::numeric_limits<float>::max();
    hittable.Intersect(local_ray, 0.001f, temp_record);
    if (temp_record.time < record.time) {
      record = temp_record;
      hit_object = component;
      hit_anything = true;
    }
  }
  
  if (hit_anything) {
    // Calculate hit point in world space
    glm::vec3 hit_pos = ray.At(record.time);
    
    // Transform normal from local to world space
    glm::mat4 local_to_world = hit_object->GetNodePtr()->GetTransform().GetLocalToWorldMatrix();
    glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(local_to_world)));
    glm::vec3 world_normal = glm::normalize(normal_matrix * record.normal);
    
    // Get material properties
    MaterialComponent* material_component = hit_object->GetNodePtr()->GetComponentPtr<MaterialComponent>();
    const Material* material;
    if (material_component != nullptr) {
      material = &material_component->GetMaterial();
    } else {
      material = &Material::GetDefault();
    }
    
    // Initialize Phong shading components
    glm::vec3 ambient_color = glm::vec3(0.0f);
    glm::vec3 diffuse_color = glm::vec3(0.0f);
    glm::vec3 specular_color = glm::vec3(0.0f);
    
    // Calculate view direction (from hit point to camera)
    glm::vec3 view_dir = glm::normalize(camera_.GetPosition() - hit_pos);
    
    // Process all lights
    for (auto& light_component : light_components_) {
      // Handle different light types
      if (light_component->GetLightPtr()->GetType() == LightType::Ambient) {
        // Ambient lighting: I_ambient = k_a * I_a
        auto ambient_light = static_cast<AmbientLight*>(light_component->GetLightPtr());
        ambient_color += ambient_light->GetAmbientColor() * material->GetAmbientColor();
      }
      else {
        // For directional and point lights, get lighting info from Illuminator
        glm::vec3 dir_to_light;
        glm::vec3 light_intensity;
        float dist_to_light;
        
        Illuminator::GetIllumination(*light_component, hit_pos, dir_to_light, light_intensity, dist_to_light);
        
        // Diffuse component: Lambertian reflection
        float diffuse_factor = glm::max(0.0f, glm::dot(world_normal, dir_to_light));
        diffuse_color += diffuse_factor * light_intensity * material->GetDiffuseColor();
        
        // Specular component: Blinn-Phong reflection
        glm::vec3 half_vector = glm::normalize(dir_to_light + view_dir);
        float specular_factor = glm::pow(glm::max(0.0f, glm::dot(world_normal, half_vector)), material->GetShininess());
        specular_color += specular_factor * light_intensity * material->GetSpecularColor();
      }
    }
    
    // Combine all lighting components
    glm::vec3 final_color = ambient_color + diffuse_color + specular_color;
    return final_color;
  }
  
  return GetBackgroundColor(ray.GetDirection());
}
