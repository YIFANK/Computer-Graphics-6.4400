#include "SplineViewerApp.hpp"

#include <fstream>

#include "gloo/cameras/ArcBallCameraNode.hpp"
#include "gloo/lights/AmbientLight.hpp"
#include "gloo/lights/PointLight.hpp"
#include "gloo/components/LightComponent.hpp"

#include "CurveNode.hpp"
#include "PatchNode.hpp"

namespace GLOO {

SplineViewerApp::SplineViewerApp(const std::string& app_name,
                                 glm::ivec2 window_size,
                                 const std::string& filename)
    : Application(app_name, window_size), filename_(filename) {
}

void SplineViewerApp::SetupScene() {
  SceneNode& root = scene_->GetRootNode();

  LoadFile(filename_, root);

  auto camera_node = make_unique<ArcBallCameraNode>();
  scene_->ActivateCamera(camera_node->GetComponentPtr<CameraComponent>());
  root.AddChild(std::move(camera_node));

  auto ambient_light = std::make_shared<AmbientLight>();
  ambient_light->SetAmbientColor(glm::vec3(0.7f));
  root.CreateComponent<LightComponent>(ambient_light);

  auto point_light = std::make_shared<PointLight>();
  point_light->SetDiffuseColor(glm::vec3(0.9f, 0.9f, 0.9f));
  point_light->SetSpecularColor(glm::vec3(1.0f, 1.0f, 1.0f));
  point_light->SetAttenuation(glm::vec3(1.0f, 0.09f, 0.032f));
  auto point_light_node = make_unique<SceneNode>();
  point_light_node->CreateComponent<LightComponent>(point_light);
  point_light_node->GetTransform().SetPosition(glm::vec3(0.0f, 4.0f, 5.f));
  root.AddChild(std::move(point_light_node));
}
bool allow_keyboard_toggle_ = true;
void SplineViewerApp::LoadFile(const std::string& filename, SceneNode& root) {
  std::fstream fs(GetAssetDir() + filename);
  if (!fs) {
    std::cerr << "ERROR: Unable to open file " + filename + "!" << std::endl;
    return;
  }

  std::string spline_type;
  std::getline(fs, spline_type);

  std::vector<glm::vec3> control_points;
  std::string line;
  for (size_t i = 0; std::getline(fs, line); i++) {
    std::stringstream ss(line);
    float x, y, z;
    ss >> x >> y >> z;
    control_points.push_back(glm::vec3(x, y, z));
  }
  for(int i = 0; i < control_points.size(); i++){
    std::cout << control_points[i].x << " " << control_points[i].y << " " << control_points[i].z << std::endl;
  }
  // TODO: set up patch or curve nodes here.
  if (spline_type == "Bezier curve") {
    if(control_points.size() % 3 != 1){
      std::cerr << "ERROR: Number of Bezier curve control points is not equal to 3n + 1" << std::endl;
    }
    if(control_points.size() > 4) {
      allow_keyboard_toggle_ = false;
    }
    for(int i = 0; i < control_points.size() - 1; i+= 3) {
      std::vector<glm::vec3> control_points_subset(control_points.begin() + i, control_points.begin() + i + 4);
      auto curve_node = make_unique<CurveNode>("Bezier",control_points_subset,allow_keyboard_toggle_);
      root.AddChild(std::move(curve_node));
    }
  } else if (spline_type == "B-Spline curve") {
    if(control_points.size() > 4) {
      allow_keyboard_toggle_ = false;
    }
    for(int i = 0; i < control_points.size() - 3; i++) {
      std::vector<glm::vec3> control_points_subset(control_points.begin() + i, control_points.begin() + i + 4);
      auto curve_node = make_unique<CurveNode>("B-Spline",control_points_subset,allow_keyboard_toggle_);
      root.AddChild(std::move(curve_node));
    }
  }else if(spline_type == "Bezier patch") {
    for(int i = 0; i < control_points.size(); i += 16){
      std::vector<glm::vec3> control_points_subset(control_points.begin() + i, control_points.begin() + i + 16);
      auto patch_node = make_unique<PatchNode>("Bezier",control_points_subset);
      root.AddChild(std::move(patch_node));
    }
  }else if(spline_type == "B-Spline patch") {
    for(int i = 0; i < control_points.size(); i += 16){
      std::vector<glm::vec3> control_points_subset(control_points.begin() + i, control_points.begin() + i + 16);
      auto patch_node = make_unique<PatchNode>("B-Spline",control_points_subset);
      root.AddChild(std::move(patch_node));
    }
  }else{
    std::cerr << "ERROR: Invalid spline type " + spline_type + "!" << std::endl;
    return;
  }
  // The first line of the user-specified file is spline_type, and the specified
  // control points are in control_points, a std::vector of glm::vec3 objects.
  // Depending on the specified spline type, create the appropriate node(s)
  // parameterized by the control points.
}
}  // namespace GLOO
