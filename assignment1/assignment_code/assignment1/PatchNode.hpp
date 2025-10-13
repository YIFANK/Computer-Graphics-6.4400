#ifndef PATCH_NODE_H_
#define PATCH_NODE_H_

#include <string>
#include <vector>

#include "gloo/SceneNode.hpp"
#include "gloo/VertexObject.hpp"
#include "gloo/shaders/ShaderProgram.hpp"

#include "CurveNode.hpp"

namespace GLOO {
struct PatchPoint {
  glm::vec3 P;
  glm::vec3 N;
};

class PatchNode : public SceneNode {
 public:
  PatchNode(std::string spline_basis, std::vector<glm::vec3> control_points);

 private:
  void InitPatch();
  void PlotPatch();
  PatchPoint EvalSurface(float u, float v);
  void ExportOBJ(const std::string& filename);
  std::vector<glm::mat4> Gs_;
  SplineBasis spline_basis_;
  std::vector<glm::vec3> control_points_;
  std::shared_ptr<VertexObject> patch_mesh_;
  std::shared_ptr<ShaderProgram> shader_;

  const int N_SUBDIV_ = 50;
  glm::mat4 B_bezier = glm::mat4(1,0,0,0,
    -3,3,0,0,
    3,-6,3,0,
    -1,3,-3,1
  );

  glm::mat4 B_bspline = (1.0f / 6.0f) * glm::mat4(1,4,1,0,
    -3,0,3,0,
    3,-6,3,0,
    -1,3,-3,1
  );
};
}  // namespace GLOO

#endif
