#include "PatchNode.hpp"

#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/shaders/PhongShader.hpp"
namespace GLOO {
PatchNode::PatchNode(std::string spline_basis, std::vector<glm::vec3> control_points) {
  shader_ = std::make_shared<PhongShader>();
  patch_mesh_ = std::make_shared<VertexObject>();
  control_points_ = control_points;
  if (spline_basis == "Bezier") {
    spline_basis_ = SplineBasis::Bezier;
  } else if (spline_basis == "B-Spline") {
    spline_basis_ = SplineBasis::BSpline;
  }
  // TODO: this node should represent a single tensor product patch.
  // Think carefully about what data defines a patch and how you can
  // render it.
  InitPatch();
}
void PatchNode::InitPatch() {
  auto shader = std::make_shared<PhongShader>();
  auto patch_node = make_unique<SceneNode>();
  patch_node->CreateComponent<ShadingComponent>(shader);
  PlotPatch();
  auto& rc = patch_node->CreateComponent<RenderingComponent>(patch_mesh_);
  rc.SetDrawMode(DrawMode::Triangles);
  AddChild(std::move(patch_node));
}
PatchPoint PatchNode::EvalSurface(float u, float v) {
  // TODO: implement evaluating the spline surface at parameter values u and v.
  PatchPoint point;
  glm::vec4 U = glm::vec4(1, u, u*u, u*u*u);
  glm::vec4 V = glm::vec4(1, v, v*v, v*v*v);
  glm::vec4 dU = glm::vec4(0, 1, 2*u, 3*u*u);
  glm::vec4 dV = glm::vec4(0, 1, 2*v, 3*v*v);
  glm::mat4 B;
  if(spline_basis_ == SplineBasis::Bezier) {
    B = B_bezier;
  } else if(spline_basis_ == SplineBasis::BSpline) {
    B = B_bspline;
  }
  glm::vec4 alpha = B * U;   // weights in u
  glm::vec4 beta  = B * V;   // weights in v

  point.P = glm::vec3(0.0f);
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      point.P += alpha[i] * beta[j] * control_points_[i*4 + j];
    }
  }
  glm::vec4 dalpha = B * dU;
  glm::vec4 dbeta  = B * dV;

  glm::vec3 Su(0.0f), Sv(0.0f);
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      glm::vec3 Pij = control_points_[i*4 + j];
      Su += dalpha[i] * beta[j] * Pij;
      Sv += alpha[i]  * dbeta[j] * Pij;
    }
  }
  glm::vec3 normal = glm::normalize(glm::cross(Su, Sv));
  point.N = -normal;
  return point;
}

void PatchNode::PlotPatch() {
  auto positions = make_unique<PositionArray>();
  auto normals = make_unique<NormalArray>();
  auto indices = make_unique<IndexArray>();

// TODO: fill "positions", "normals", and "indices"
  for(int i = 0; i <= N_SUBDIV_; i++) {
    for(int j = 0; j <= N_SUBDIV_; j++) {
      auto point = EvalSurface(static_cast<float>(i) / N_SUBDIV_, static_cast<float>(j) / N_SUBDIV_);
      positions->push_back(point.P);
      normals->push_back(point.N);
    }
  }
  //construct triangular meshes
  for(int i = 0; i < N_SUBDIV_; i++) {
    for(int j = 0; j < N_SUBDIV_; j++) {
      int id1 = i * (N_SUBDIV_ + 1) + j;
      int id2 = i * (N_SUBDIV_ + 1) + j + 1;
      int id3 = (i + 1) * (N_SUBDIV_ + 1) + j;
      int id4 = (i + 1) * (N_SUBDIV_ + 1) + j + 1;
      indices->push_back(id1);
      indices->push_back(id2);
      indices->push_back(id3);
      indices->push_back(id2);
      indices->push_back(id3);
      indices->push_back(id4);
    }
  }
  patch_mesh_->UpdatePositions(std::move(positions));
  patch_mesh_->UpdateNormals(std::move(normals));
  patch_mesh_->UpdateIndices(std::move(indices));
}
// void PatchNode::ExportOBJ(const std::string& filename) {
//   std::ofstream out(filename);
//   if (!out) return;
//   for(auto& pos : patch_mesh_->GetPositions()) {
//     out << "v " << pos.x << " " << pos.y << " " << pos.z << std::endl;
//   }
//   for(auto& norm : patch_mesh_->GetNormals()) {
//     out << "vn " << norm.x << " " << norm.y << " " << norm.z << std::endl;
//   }
//   for(int i = 0; i < patch_mesh_->GetIndices().size(); i += 3) {
//     int i0 = patch_mesh_->GetIndices()[i];
//     int i1 = patch_mesh_->GetIndices()[i+1];
//     int i2 = patch_mesh_->GetIndices()[i+2];
//     out << "f " << i0 << "//" << i0 << " " << i1 << "//" << i1 << " " << i2 << "//" << i2 << std::endl;
//   }
// }
}  // namespace GLOO
