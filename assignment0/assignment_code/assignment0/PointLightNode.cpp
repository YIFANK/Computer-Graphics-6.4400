#include "PointLightNode.hpp"
// More include here
#include "gloo/external.hpp"
#include "glm/gtx/string_cast.hpp"

#include "gloo/shaders/PhongShader.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/CameraComponent.hpp"
#include "gloo/components/LightComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/MeshLoader.hpp"
#include "gloo/lights/PointLight.hpp"
#include "gloo/lights/AmbientLight.hpp"
#include "gloo/cameras/BasicCameraNode.hpp"
#include "gloo/InputManager.hpp"
namespace GLOO {
PointLightNode::PointLightNode() {
    auto point_light = std::make_shared<PointLight>();
    point_light->SetDiffuseColor(glm::vec3(0.8f, 0.8f, 0.8f));
    point_light->SetSpecularColor(glm::vec3(1.0f, 1.0f, 1.0f));
    point_light->SetAttenuation(glm::vec3(1.0f, 0.09f, 0.032f));
    CreateComponent<LightComponent>(point_light);
    GetTransform().SetPosition(glm::vec3(0.0f, 4.0f, 5.f));
}
void PointLightNode::Update(double delta_time) {
// Update
    //update the position of the point light using the arrow keys
    //262-265 is the arrow key for right, left, down, up
    auto point_light = GetComponentPtr<LightComponent>();
    if (point_light == nullptr) return;
    auto position = GetTransform().GetPosition();
    if (InputManager::GetInstance().IsKeyPressed(262)) {
        GetTransform().SetPosition(position + glm::vec3(0.1f, 0.0f, 0.0f));
    }
    if (InputManager::GetInstance().IsKeyPressed(263)) {
        GetTransform().SetPosition(position + glm::vec3(-0.1f, 0.0f, 0.0f));
    }
    if (InputManager::GetInstance().IsKeyPressed(264)) {
        GetTransform().SetPosition(position + glm::vec3(0.0f, -0.1f, 0.0f));
    }
    if (InputManager::GetInstance().IsKeyPressed(265)) {
        GetTransform().SetPosition(position + glm::vec3(0.0f, 0.1f, 0.0f));
    }
    // if (InputManager::GetInstance().IsKeyPressed(GLFW_KEY_I)) {
    //     GetTransform().SetPosition(position + glm::vec3(0.0f, 0.0f, 0.1f));
    // }
    // if (InputManager::GetInstance().IsKeyPressed(GLFW_KEY_K)) {
    //     GetTransform().SetPosition(position + glm::vec3(0.0f, 0.0f, -0.1f));
    // }
    return;
}

}