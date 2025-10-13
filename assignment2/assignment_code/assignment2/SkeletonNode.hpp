#ifndef SKELETON_NODE_H_
#define SKELETON_NODE_H_

#include "gloo/SceneNode.hpp"
#include "gloo/VertexObject.hpp"
#include "gloo/shaders/ShaderProgram.hpp"
#include "gloo/shaders/PhongShader.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"
#include <string>
#include <vector>

namespace GLOO {
class SkeletonNode : public SceneNode {
 public:
  enum class DrawMode { Skeleton, SSD };
  struct EulerAngle {
    float rx, ry, rz;
  };

  SkeletonNode(const std::string& filename);
  void LinkRotationControl(const std::vector<EulerAngle*>& angles);
  void Update(double delta_time) override;
  void OnJointChanged();

 private:
  void LoadAllFiles(const std::string& prefix);
  void LoadSkeletonFile(const std::string& path);
  void LoadMeshFile(const std::string& filename);
  void LoadAttachmentWeights(const std::string& path);

  void ToggleDrawMode();
  void DecorateTree();
  //joints
  std::shared_ptr<VertexObject> vtx_obj;
  SceneNode* skin_node;
  std::vector<SceneNode*> joint_nodes_,sphere_nodes_,cylinder_nodes_;
  std::vector<std::pair<int,int>> edges;
  std::vector <glm::vec3> bind_pose_positions,bind_pose_normals;
  std::vector<glm::mat4> inv_bind_pose_matrices;
  std::vector<std::vector<float>> weights;
  std::shared_ptr<PhongShader> shader_;
  std::shared_ptr<VertexObject> sphere_mesh_,cylinder_mesh_;
  DrawMode draw_mode_;
  // Euler angles of the UI sliders.
  std::vector<EulerAngle*> linked_angles_;
};
}  // namespace GLOO

#endif
