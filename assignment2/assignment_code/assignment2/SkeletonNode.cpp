#include "SkeletonNode.hpp"

#include "gloo/utils.hpp"
#include "gloo/InputManager.hpp"
#include "gloo/MeshLoader.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include <fstream>
#include <sstream>

namespace GLOO {
SkeletonNode::SkeletonNode(const std::string& filename)
    : SceneNode(), draw_mode_(DrawMode::Skeleton) {
  std::cerr << "SkeletonNode constructor: " << filename << std::endl;
  shader_ = std::make_shared<PhongShader>();
  sphere_mesh_ = PrimitiveFactory::CreateSphere(0.02f, 25, 25);
  cylinder_mesh_ = PrimitiveFactory::CreateCylinder(0.01f, 1.0f, 25);
  LoadAllFiles(filename);
  DecorateTree();

  // Force initial update.
  OnJointChanged();
}

void SkeletonNode::ToggleDrawMode() {
  draw_mode_ =
      draw_mode_ == DrawMode::Skeleton ? DrawMode::SSD : DrawMode::Skeleton;
  // TODO: implement here toggling between skeleton mode and SSD mode.
  // The current mode is draw_mode_;
  // Hint: you may find SceneNode::SetActive convenient here as
  // inactive nodes will not be picked up by the renderer.
  skin_node->SetActive(draw_mode_ == DrawMode::SSD);
  //make all sphere nodes and sphere nodes active when draw mode is skeleton
  for(int i = 0;i < sphere_nodes_.size();i++){
    sphere_nodes_[i]->SetActive(draw_mode_ == DrawMode::Skeleton);
  }
  for(int i = 0;i < cylinder_nodes_.size();i++){
    cylinder_nodes_[i]->SetActive(draw_mode_ == DrawMode::Skeleton);
  }
}

void SkeletonNode::DecorateTree() {
  // TODO: set up addtional nodes, add necessary components here.
  // You should create one set of nodes/components for skeleton mode
  // (spheres for joints and cylinders for bones), and another set for
  // SSD mode (you could just use a single node with a RenderingComponent
  // that is linked to a VertexObject with the mesh information. Then you
  // only need to update the VertexObject - updating vertex positions and
  // recalculating the normals, etc.).

  // The code snippet below shows how to add a sphere node to a joint.
  // Suppose you have created member variables shader_ of type
  // std::shared_ptr<PhongShader>, and sphere_mesh_ of type
  // std::shared_ptr<VertexObject>.
  // Here sphere_nodes_ptrs_ is a std::vector<SceneNode*> that stores the
  // pointer so the sphere nodes can be accessed later to change their
  // positions. joint_ptr is assumed to be one of the joint node you created
  // from LoadSkeletonFile (e.g. you've stored a std::vector<SceneNode*> of
  // joint nodes as a member variable and joint_ptr is one of the elements).
  std::cerr << "Decorating tree: " << joint_nodes_.size() << std::endl;
  //add sphere to each joint node
  //add cylinder in between each joint node
  for(int i = 0; i < joint_nodes_.size(); i++) {
    for(int j = 0; j < joint_nodes_[i]->GetChildrenCount(); j++) {
      std::cerr << "Adding cylinder between joint " << i << " and child " << j << std::endl;
      auto cylinder_node = make_unique<SceneNode>();
      cylinder_node->CreateComponent<ShadingComponent>(shader_);
      cylinder_node->CreateComponent<RenderingComponent>(cylinder_mesh_);
      cylinder_nodes_.push_back(cylinder_node.get());
      AddChild(std::move(cylinder_node));
    }
  }
  for (int i = 0; i < joint_nodes_.size(); i++) {
    auto sphere_node = make_unique<SceneNode>();
    sphere_node->CreateComponent<ShadingComponent>(shader_);
    sphere_node->CreateComponent<RenderingComponent>(sphere_mesh_);
    sphere_nodes_.push_back(sphere_node.get());
    sphere_nodes_[i]->GetTransform().SetPosition(glm::vec3(0.f,0.f,0.f));
    joint_nodes_[i]->AddChild(std::move(sphere_node));
    inv_bind_pose_matrices.push_back(glm::inverse(joint_nodes_[i]->GetTransform().GetLocalToWorldMatrix()));
  }
  //initializing skin
  auto skin_node_unique = make_unique<SceneNode>();
  skin_node_unique->CreateComponent<ShadingComponent>(shader_);
  auto &skin_rc = skin_node_unique->CreateComponent<RenderingComponent>(vtx_obj);
  skin_rc.SetDrawMode(GLOO::DrawMode::Triangles);
  //change color to grey
  auto white = glm::vec3(1.f,1.f,1.f);
  auto material = std::make_shared<Material>(
        white,   // diffuse
        white,   // ambient
        white,   // specular
        0.0f                        // shininess
    );
  skin_node_unique->CreateComponent<MaterialComponent>(material);
  skin_node = skin_node_unique.get();
  skin_node->SetActive(draw_mode_ == DrawMode::SSD);
  AddChild(std::move(skin_node_unique));
}

void SkeletonNode::Update(double delta_time) {
  // Prevent multiple toggle.
  static bool prev_released = true;
  if (InputManager::GetInstance().IsKeyPressed('S')) {
    if (prev_released) {
      ToggleDrawMode();
    }
    prev_released = false;
  } else if (InputManager::GetInstance().IsKeyReleased('S')) {
    prev_released = true;
  }
}

void SkeletonNode::OnJointChanged() {
  // TODO: this method is called whenever the values of UI sliders change.
  // The new Euler angles (represented as EulerAngle struct) can be retrieved
  // from linked_angles_ (a std::vector of EulerAngle*).
  // The indices of linked_angles_ align with the order of the joints in .skel
  // files. For instance, *linked_angles_[0] corresponds to the first line of
  // the .skel file.
  for(int i = 0;i < linked_angles_.size();i++){
    auto &angle = linked_angles_[i];
    //update the new Euler angles
    auto q = glm::quat(glm::vec3(angle->rx,angle->ry,angle->rz));
    joint_nodes_[i]->GetTransform().SetRotation(q);
  }
  //recompute the bones
  for(int i = 0;i < edges.size();i++){
    int x = edges[i].first,y = edges[i].second;
    //set new position and rotation
    glm::vec3 p1 = joint_nodes_[x]->GetTransform().GetWorldPosition();
    glm::vec3 p2 = joint_nodes_[y]->GetTransform().GetWorldPosition();
    glm::vec3 dir = p2 - p1,u = glm::vec3(0,1,0);
    auto axis = glm::normalize(glm::cross(dir,u));
    float angle = acos(glm::dot(dir,u) / glm::length(dir));
    auto &transform = cylinder_nodes_[i]->GetTransform();
    transform.SetPosition(p1);
    transform.SetRotation(axis,-angle);
    transform.SetScale(glm::vec3(1.f, glm::length(p1 - p2), 1.f));
  }
  //TODO: deforming mesh
    std::vector <glm::vec3> normals,positions;
    int vertice_size = vtx_obj->GetPositions().size();
    normals.resize(vertice_size);
    auto indices = vtx_obj->GetIndices();
    //std::cerr << "Deforming mesh" << std::endl;
    //calculate vertex positions
    for(int i = 0;i < vertice_size;i++){
      glm::vec4 p(0.f,0.f,0.f,0.f);
      for(int j = 0;j < joint_nodes_.size();j++){
        float weight = weights[i][j];
        //calculate vertex position wrt to joint j
        glm::mat4 T = joint_nodes_[j]->GetTransform().GetLocalToWorldMatrix();
        auto B = inv_bind_pose_matrices[j];
        p += weight * T * B * glm::vec4(bind_pose_positions[i],1.f) ;
      }
      positions.push_back(glm::vec3(p));
    }
    auto positions_array = make_unique<PositionArray>();
    for(int i = 0;i < vertice_size;i++){
      positions_array->push_back(positions[i]);
    }
    vtx_obj->UpdatePositions(std::move(positions_array));
    //calculate vertex normals
    for(int i = 0;i < indices.size();i += 3){
      int a = indices[i],b = indices[i+1],c = indices[i+2];
      //calculate face normal
      glm::vec3 n = glm::cross(positions[b] - positions[a],positions[c] - positions[a]);
      normals[a] += n;
      normals[b] += n;
      normals[c] += n;
    }
    for(int i = 0;i < vertice_size;i++){
      normals[i] = glm::normalize(normals[i]);
    }
    auto normals_array = make_unique<NormalArray>();
    for(int i = 0;i < vertice_size;i++){
      normals_array->push_back(normals[i]);
    }
    vtx_obj->UpdateNormals(std::move(normals_array));
}

void SkeletonNode::LinkRotationControl(const std::vector<EulerAngle*>& angles) {
  linked_angles_ = angles;
}

void SkeletonNode::LoadSkeletonFile(const std::string& path) {
  // TODO: load skeleton file and build the tree of joints.
  std::ifstream file(path);
  std::string line;
  //std::cerr << "Loading skeleton file: " << path << std::endl;
  int now = 0;
  while (std::getline(file, line)) {
    std::istringstream iss(line);
    int parent;
    float x,y,z;
    iss >> x >> y >> z >> parent;
    auto joint_node = make_unique<SceneNode>();
    joint_node->GetTransform().SetPosition(glm::vec3(x,y,z));
    joint_nodes_.push_back(joint_node.get());
    //std::cerr << "Adding joint node: " << joint_nodes_.size() << std::endl;
    if (parent != -1) {
      edges.emplace_back(parent,now);
      joint_nodes_[parent]->AddChild(std::move(joint_node));
    }
    else{
      AddChild(std::move(joint_node));
    }
    now++;
  }
}

void SkeletonNode::LoadMeshFile(const std::string& filename) {
  vtx_obj =
      MeshLoader::Import(filename).vertex_obj;
  // TODO: store the bind pose mesh in your preferred way.
  //initialize normals
  auto normals = make_unique<NormalArray>();
  for(int i = 0;i < vtx_obj->GetPositions().size();i++){
    bind_pose_positions.push_back(vtx_obj->GetPositions()[i]);
    normals->push_back(glm::vec3(0.f,0.f,0.f));
  }
  vtx_obj->UpdateNormals(std::move(normals));
}

void SkeletonNode::LoadAttachmentWeights(const std::string& path) {
  // TODO: load attachment weights.
  std::ifstream file(path);
  std::string line;
  int i = 0;
  weights.resize(vtx_obj->GetPositions().size());
  while (std::getline(file, line)) {
    std::istringstream iss(line);
    weights[i].resize(joint_nodes_.size());
    for(int j = 1;j < joint_nodes_.size();j++){
      float weight;
      iss >> weight;
      weights[i][j] = weight;
    }
    i++;
  }
}

void SkeletonNode::LoadAllFiles(const std::string& prefix) {
  std::string prefix_full = GetAssetDir() + prefix;
  LoadSkeletonFile(prefix_full + ".skel");
  LoadMeshFile(prefix + ".obj");
  LoadAttachmentWeights(prefix_full + ".attach");
}
}  // namespace GLOO
