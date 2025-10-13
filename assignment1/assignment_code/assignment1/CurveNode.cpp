#include "CurveNode.hpp"

#include "gloo/debug/PrimitiveFactory.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/shaders/PhongShader.hpp"
#include "gloo/shaders/SimpleShader.hpp"
#include "gloo/InputManager.hpp"


namespace GLOO {
CurveNode::CurveNode(std::string spline_basis, std::vector<glm::vec3> control_points,bool allow_keyboard_toggle) {
  // TODO: this node should represent a single spline curve.
  // Think carefully about what data defines a curve and how you can
  // render it.

  // Initialize the VertexObjects and shaders used to render the control points,
  // the curve, and the tangent line.
  allow_keyboard_toggle_ = allow_keyboard_toggle;
  sphere_mesh_ = PrimitiveFactory::CreateSphere(0.015f, 25, 25);
  curve_polyline_ = std::make_shared<VertexObject>(); 
  control_points_ = control_points;
  tangent_line_ = std::make_shared<VertexObject>();
  shader_ = std::make_shared<PhongShader>();
  polyline_shader_ = std::make_shared<SimpleShader>();
  // Initialize the spline basis to Bezier or B-Spline
  if (spline_basis == "Bezier") {
    spline_basis_ = SplineBasis::Bezier;
  } else if (spline_basis == "B-Spline") {
    spline_basis_ = SplineBasis::BSpline;
  }
  InitCurve();
  PlotCurve();
}

void CurveNode::Update(double delta_time) {
  if(!allow_keyboard_toggle_) {
    return;
  }
  // Prevent multiple toggle.
  static bool prev_released = true;

  if (InputManager::GetInstance().IsKeyPressed('T')) {
    if (prev_released) {
      // TODO: implement toggling spline bases.
      ToggleSplineBasis();
    }
    prev_released = false;
  } else if (InputManager::GetInstance().IsKeyPressed('B')) {
    if (prev_released) {
      // TODO: implement converting conrol point geometry from Bezier to
      // B-Spline basis.
      ConvertGeometry(false);
    }
    prev_released = false;
  } else if (InputManager::GetInstance().IsKeyPressed('Z')) {
    if (prev_released) {
      // TODO: implement converting conrol point geometry from B-Spline to
      // Bezier basis.
      ConvertGeometry(true);
    }
    prev_released = false;
  } else {
    prev_released = true;
  }
  PlotCurve();
  PlotTangentLine();
}

void CurveNode::ToggleSplineBasis() {
  // TODO: implement toggling between Bezier and B-Spline bases.
  if (spline_basis_ == SplineBasis::Bezier) {
    spline_basis_ = SplineBasis::BSpline;
    //change color of control points to green
    for (auto& control_node : control_nodes_) {
      auto &material = control_node->GetComponentPtr<MaterialComponent>()->GetMaterial();
      glm::vec3 new_color = glm::vec3(0.f, 1.f, 0.f);
      material.SetDiffuseColor(new_color);
      material.SetSpecularColor(new_color);
      material.SetAmbientColor(new_color);
    }
  } else {
    spline_basis_ = SplineBasis::Bezier;
    //change color of control points to red
    for (auto& control_node : control_nodes_) {
      auto &material = control_node->GetComponentPtr<MaterialComponent>()->GetMaterial();
      glm::vec3 new_color = glm::vec3(1.f, 0.f, 0.f);
      material.SetDiffuseColor(new_color);
      material.SetSpecularColor(new_color);
      material.SetAmbientColor(new_color);
    }
  }
}

void CurveNode::ConvertGeometry(bool bezier) {
  // TODO: implement converting the control points between bases.
  glm::mat4 M;
  if (bezier) {
    M = B_bspline * glm::inverse(B_bezier) ;
  }
  else{
    M = B_bezier * glm::inverse(B_bspline);
  }
  M = glm::transpose(M);
  // Gather control points
  glm::vec4 X, Y, Z;
  for (int i = 0; i < 4; i++) {
      glm::vec3 pos = control_nodes_[i]->GetTransform().GetPosition();
      X[i] = pos.x;
      Y[i] = pos.y;
      Z[i] = pos.z;
  }

  // Transform each coordinate
  glm::vec4 X_new = M * X;
  glm::vec4 Y_new = M * Y;
  glm::vec4 Z_new = M * Z;

  // Update back into control nodes
  for (int i = 0; i < 4; i++) {
      control_nodes_[i]->GetTransform().SetPosition(
          glm::vec3(X_new[i], Y_new[i], Z_new[i])
      );
  }
}

CurvePoint CurveNode::EvalCurve(float t) {
  // TODO: implement evaluating the spline curve at parameter value t.
  // glm::vec4 T(t*t*t, t*t, t, 1.0f);
  // glm::vec4 dT(3*t*t, 2*t, 1.0f, 0.0f);
  glm::vec4 T(1, t, t*t, t*t*t);
  glm::vec4 dT(0, 1, 2*t, 3*t*t);
  glm::mat<4,3,float> P;
  CurvePoint result;
  P[0] = control_nodes_[0]->GetTransform().GetPosition();
  P[1] = control_nodes_[1]->GetTransform().GetPosition();
  P[2] = control_nodes_[2]->GetTransform().GetPosition();
  P[3] = control_nodes_[3]->GetTransform().GetPosition();
  glm::mat4 B;
  if (spline_basis_ == SplineBasis::Bezier) {
    B = B_bezier;
  }
  else{
    B = B_bspline;
  }
  glm::vec4 coeffs = B * T;   // (4x4 * vec4) = vec4

  // Linear combination
  glm::vec3 point(0.0f);
  for (int i = 0; i < 4; i++) {
      point += coeffs[i] * P[i];  // cps[i] is glm::vec3
  }
  glm::vec3 tangent(0.0f);
  coeffs = B * dT;
  for(int i = 0; i < 4; i++) {
    tangent += coeffs[i] * P[i];
  }
  result.P = point;
  result.T = tangent;
  return result;
}

void CurveNode::InitCurve() {
  // TODO: create all of the nodes and components necessary for rendering the 
  // curve, its control points, and its tangent line. You will want to use the 
  // VertexObjects and shaders that are initialized in the class
  for (auto& pos : control_points_) {
    auto control_point_node = make_unique<SceneNode>();
    control_point_node->CreateComponent<ShadingComponent>(shader_);
    control_point_node->CreateComponent<RenderingComponent>(sphere_mesh_);
    glm::vec3 node_color;
    if(spline_basis_ == SplineBasis::Bezier) {
      node_color = glm::vec3(1.f, 0.f, 0.f);
    }
    else{
      node_color = glm::vec3(0.f, 1.f, 0.f);
    }
    auto material = std::make_shared<Material>(
        node_color,   // diffuse
        node_color,   // ambient
        glm::vec3(1.f, 0.f, 0.f),   // specular
        0.0f                        // shininess
    );
    control_point_node->CreateComponent<MaterialComponent>(material);
    control_point_node->GetTransform().SetPosition(pos);

    control_nodes_.push_back(control_point_node.get());  // keep pointer for later edits
    AddChild(std::move(control_point_node));             // scene graph owns it
  }

  // --- Curve Line ---
  curve_polyline_ = std::make_shared<VertexObject>();
  PlotCurve();
  auto shader = std::make_shared<SimpleShader>();
  auto line_node = make_unique<SceneNode>();
  line_node->CreateComponent<ShadingComponent>(shader);

  auto& rc = line_node->CreateComponent<RenderingComponent>(curve_polyline_);
  rc.SetDrawMode(DrawMode::Lines);

  glm::vec3 color(1.f, 1.f, 0.f);
  auto material = std::make_shared<Material>(color, color, color, 0);
  line_node->CreateComponent<MaterialComponent>(material);

  AddChild(std::move(line_node));

  // --- Tangent Line (or placeholder) ---
  PlotTangentLine();
  auto tangent_line_node_ = make_unique<SceneNode>();
  tangent_line_node_->CreateComponent<ShadingComponent>(shader);

  auto &tangent_line_rc = tangent_line_node_->CreateComponent<RenderingComponent>(tangent_line_);
  tangent_line_rc.SetDrawMode(DrawMode::Lines);

  //make white
  color = glm::vec3(1.f, 1.f, 1.f);
  material = std::make_shared<Material>(color, color, color, 0);
  tangent_line_node_->CreateComponent<MaterialComponent>(material);
  AddChild(std::move(tangent_line_node_));
}

void CurveNode::PlotCurve() {
  // Only update the geometry, not the node
  auto positions = make_unique<PositionArray>();
  for (int i = 0; i <= N_SUBDIV_; i++) {
    float t = i / static_cast<float>(N_SUBDIV_);
    auto curve_point = EvalCurve(t);
    positions->push_back(curve_point.P);
  }

  auto indices = make_unique<IndexArray>();
  for (size_t i = 0; i < positions->size() - 1; i++) {
    indices->push_back(i);
    indices->push_back(i + 1);
  }

  curve_polyline_->UpdatePositions(std::move(positions));
  curve_polyline_->UpdateIndices(std::move(indices));
}


void CurveNode::PlotControlPoints() {
  // TODO: plot the curve control points.
  for (auto& control_point : control_points_) {
    auto control_point_node = make_unique<SceneNode>();
    control_point_node->CreateComponent<ShadingComponent>(shader_);
    control_point_node->CreateComponent<RenderingComponent>(sphere_mesh_);
    control_point_node->CreateComponent<MaterialComponent>(std::make_shared<Material>(glm::vec3(1.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 0.f), 0.0f));
    control_point_node->GetTransform().SetPosition(control_point);
    control_nodes_.push_back(control_point_node.get());
    AddChild(std::move(control_point_node));
  }
}

void CurveNode::PlotTangentLine() {
  // TODO: implement plotting a line tangent to the curve.
  // Below is a sample implementation for rendering a line segment
  // onto the screen. Note that this is just an example. This code
  // currently has nothing to do with the spline.
  std::unique_ptr<PositionArray> positions = make_unique<PositionArray>();
  float t = 0.5;
  auto curve_point = EvalCurve(t);
  positions->push_back(curve_point.P - 0.5f * curve_point.T);
  positions->push_back(curve_point.P + 0.5f * curve_point.T);
  auto indices = make_unique<IndexArray>();
  indices->push_back(0);
  indices->push_back(1);
  tangent_line_->UpdatePositions(std::move(positions));
  tangent_line_->UpdateIndices(std::move(indices));
}
}  // namespace GLOO
