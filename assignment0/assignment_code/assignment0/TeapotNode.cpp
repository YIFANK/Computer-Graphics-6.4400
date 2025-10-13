#include "TeapotNode.hpp"
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
TeapotNode::TeapotNode() {
    std::shared_ptr<PhongShader> shader = std::make_shared<PhongShader>();
    std::shared_ptr<VertexObject> mesh =
        MeshLoader::Import("assignment0/teapot.obj").vertex_obj;
    if (mesh == nullptr) {
    return;
    }
    CreateComponent<ShadingComponent>(shader);
    CreateComponent<RenderingComponent>(mesh);
    CreateComponent<MaterialComponent>(
        std::make_shared<Material>(Material::GetDefault()));
    GetTransform().SetPosition(glm::vec3(0.f, 0.f, 0.f));
    GetTransform().SetRotation(glm::vec3(1.0f, 0.0f, 0.0f), 0.3f);
}
void TeapotNode::ToggleColor() {
    auto material_component = GetComponentPtr<MaterialComponent>();
    if (material_component == nullptr) return;

    auto& material = material_component->GetMaterial();

    // Define a palette of colors
    static std::vector<glm::vec3> colors = {
        {0.5f, 0.1f, 0.2f},  // dark red (default)
        {0.1f, 0.5f, 0.2f},  // green
        {0.2f, 0.3f, 0.8f},  // blue
        {0.8f, 0.8f, 0.2f}   // yellow
    };

    // Cycle through
    static int color_index_ = 0;
    color_index_ = (color_index_ + 1) % colors.size();

    glm::vec3 new_color = colors[color_index_];
    material.SetDiffuseColor(new_color);
    material.SetSpecularColor(new_color);
    material.SetAmbientColor(new_color);
}
void TeapotNode::ChangeColor(){
    auto material_component = GetComponentPtr<MaterialComponent>();
    if (material_component == nullptr) return;
    // Define a palette of colors
    static std::vector<glm::vec3> colors = {
        {0.5f, 0.1f, 0.2f},  // dark red (default)
        {0.1f, 0.5f, 0.2f},  // green
        {0.2f, 0.3f, 0.8f},  // blue
        {0.8f, 0.8f, 0.2f}   // yellow
    };
    auto& material = material_component->GetMaterial();
    auto old_color = material.GetDiffuseColor();
    static int color_index_ = 0;
    static int count = 0;
    int new_color_index = (color_index_ + 1) % colors.size();
    //update the color index
    if(count == 100){
        color_index_ = new_color_index;
        new_color_index = (color_index_ + 1) % colors.size();
        count = 0;
    }
    glm::vec3 d = (colors[new_color_index] - colors[color_index_])/100.0f;
    count++;
    material.SetDiffuseColor(old_color + d);
    material.SetSpecularColor(old_color + d);
    material.SetAmbientColor(old_color + d);
    //check if we have reached the new color
}
void TeapotNode::SpinModel(){
    auto &transform = GetTransform();
    static float angle = 0.05f;
    auto q = glm::angleAxis(angle, glm::vec3(0, 1, 0));
    transform.SetRotation(q * transform.GetRotation());
}
void TeapotNode::Update(double delta_time) {
// Update
    if(InputManager::GetInstance().IsKeyPressed(GLFW_KEY_C)){
        ChangeColor();
    }
    if(InputManager::GetInstance().IsKeyPressed(GLFW_KEY_R)){
        //spin the model
        //std::cout << "spinning the model\n";
        SpinModel();
    }
}

}