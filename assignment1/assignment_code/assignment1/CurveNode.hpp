#ifndef CURVE_NODE_H_
#define CURVE_NODE_H_

#include <string>
#include <vector>

#include "gloo/SceneNode.hpp"
#include "gloo/VertexObject.hpp"
#include "gloo/shaders/ShaderProgram.hpp"

namespace GLOO {

enum class SplineBasis { Bezier, BSpline };

struct CurvePoint {
  glm::vec3 P;
  glm::vec3 T;
};

class CurveNode : public SceneNode {
 public:
  CurveNode(std::string spline_basis, std::vector<glm::vec3> control_points,bool allow_keyboard_toggle);
  void Update(double delta_time) override;

 private:
  void ToggleSplineBasis();
  void ConvertGeometry(bool bezier);
  CurvePoint EvalCurve(float t);
  void InitCurve();
  void PlotCurve();
  void PlotControlPoints();
  void PlotTangentLine();
  bool allow_keyboard_toggle_;
  SplineBasis spline_basis_;
  std::vector<glm::vec3> control_points_;
  std::vector<SceneNode*> control_nodes_;
  std::shared_ptr<VertexObject> sphere_mesh_;
  std::shared_ptr<VertexObject> curve_polyline_;
  std::shared_ptr<VertexObject> tangent_line_;

  std::shared_ptr<ShaderProgram> shader_;
  std::shared_ptr<ShaderProgram> polyline_shader_;


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
