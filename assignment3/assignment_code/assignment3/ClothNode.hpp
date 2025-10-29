#pragma once
#include "gloo/SceneNode.hpp"
#include "ForwardEulerIntegrator.hpp"
#include "RK4Integrator.hpp"
#include "TrapezoidalIntegrator.hpp"
#include "IntegratorFactory.hpp"
#include "PendulumSystem.hpp"
#include "gloo/shaders/PhongShader.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/InputManager.hpp"
#include <random>
namespace GLOO {

class ClothNode : public SceneNode {
 public:
  ClothNode(IntegratorType integrator_type, float dt,int width, int height)
      : m_dt(dt), m_time(0.0f), m_width(width), m_height(height), gentle_wind(false) {
    //initialize the cloth system
    glm::vec3 origin = glm::vec3(2.0f, 2.0f, 0.0f);
    float d = 0.2f;
    for (int i = 0; i < m_width; i++) {
      for (int j = 0; j < m_height; j++) {
        m_system.AddParticle(origin + glm::vec3(-j * d, -i * d, 0.0f), 0.1f);
      }
    }
    //structural springs
    for (int i = 0; i < m_width; i++) {
      for (int j = 0; j < m_height; j++) {
        if (j < m_height - 1) {
          m_system.AddSpring(i * m_height + j, i * m_height + j + 1, 100.0f, d);
        }
        if (i < m_width - 1) {
          m_system.AddSpring(i * m_height + j, (i + 1) * m_height + j, 100.0f, d);
        }
      }
    }
    //shear springs
    for (int i = 0; i < m_width; i++) {
      for (int j = 0; j < m_height; j++) {
        if (i < m_width - 1 && j < m_height - 1) {
          m_system.AddSpring(i * m_height + j, (i + 1) * m_height + j + 1, 100.0f, sqrt(2.0f) * d);
        }
        if (i < m_width - 1 && j > 0) {
          m_system.AddSpring(i * m_height + j, (i + 1) * m_height + j - 1, 100.0f, sqrt(2.0f) * d);
        }
      }
    }
    //flex springs
    for (int i = 0; i < m_width; i++) {
      for (int j = 0; j < m_height; j++) {
        if (i < m_width - 2) {
          m_system.AddSpring(i * m_height + j, (i + 2) * m_height + j, 1000.0f, 2.0f * d);
        }
        if (j < m_height - 2) {
          m_system.AddSpring(i * m_height + j, i * m_height + j + 2, 1000.0f, 2.0f * d);
        }
      }
    }
    //fix the top two corners
    m_system.FixParticle(0);
    m_system.FixParticle(m_width - 1);
    // Get initial state from the system
    m_state = m_system.GetInitialState();

    // create integrator
    m_integrator = IntegratorFactory::CreateIntegrator<PendulumSystem, ParticleState>(
        integrator_type);
    
    shader = std::make_shared<PhongShader>();
    // initialize counts and mesh
    m_num_particles = m_width * m_height;
    mesh = std::make_shared<VertexObject>();
    //create the cloth mesh
    auto positions = make_unique<PositionArray>();
    auto indices = make_unique<IndexArray>();
    for (int i = 0; i < m_width; i++) {
      for (int j = 0; j < m_height; j++) {
        positions->push_back(m_state.positions[i * m_height + j]);
      }
    }
    //draw triangles 
    for (int i = 0; i < m_width - 1; i++) {
      for (int j = 0; j < m_height - 1; j++) {
        int id1 = i * m_height + j;
        int id2 = i * m_height + j + 1;
        int id3 = (i + 1) * m_height + j;
        int id4 = (i + 1) * m_height + j + 1;
        indices->push_back(id1);
        indices->push_back(id2);
        indices->push_back(id3);
        indices->push_back(id2);
        indices->push_back(id4);
        indices->push_back(id3);
      }
    }
    auto normals = ComputeNormals();
    mesh->UpdatePositions(std::move(positions));
    mesh->UpdateIndices(std::move(indices));
    mesh->UpdateNormals(std::move(normals));
    std::cerr << "Created cloth mesh" << std::endl;
    //create the cloth mesh by drawing structural springs
    auto &rc = CreateComponent<RenderingComponent>(mesh);
    rc.SetDrawMode(DrawMode::Triangles);
    CreateComponent<ShadingComponent>(shader);
  }
  ParticleState random_force(const ParticleState& state) const {
      ParticleState derivative;
      int n = state.positions.size();
      derivative.positions.resize(n);
      derivative.velocities.resize(n);
      return derivative;
      for (int i = 0; i < n; i++) {
          derivative.positions[i] = state.velocities[i];
          //generate a random vector
          std::random_device rd;
          std::mt19937 gen(rd());
          std::uniform_real_distribution<> dis(0.0f, 10.0f);
          glm::vec3 random_vector = glm::vec3(dis(gen), dis(gen), dis(gen));
          derivative.velocities[i] = random_vector;
      }
      return derivative;
  }

  void Update(double delta_time) override {
    if (InputManager::GetInstance().IsKeyPressed('R')) {
      Reset();
    }
    if (InputManager::GetInstance().IsKeyPressed('T')) {
      gentle_wind = !gentle_wind;
    }
    int steps = static_cast<int>(delta_time / static_cast<double>(m_dt));
    for (int i = 0; i < steps; ++i) {
      m_state = m_integrator->Integrate(m_system, m_state, m_time, m_dt);
      if (gentle_wind) {
        //std::cerr << "Applying gentle wind" << std::endl;
        m_state += random_force(m_state);
      }
      m_time += m_dt;
    }
    // update mesh vertex positions from the simulated state
    auto positions = make_unique<PositionArray>();
    positions->reserve(static_cast<size_t>(m_num_particles));
    for (int i = 0; i < m_width; i++) {
      for (int j = 0; j < m_height; j++) {
        positions->push_back(m_state.positions[i * m_height + j]);
      }
    }
    mesh->UpdatePositions(std::move(positions));
    //update the normals
    auto normals = ComputeNormals();
    mesh->UpdateNormals(std::move(normals));
  }
  std::unique_ptr<NormalArray> ComputeNormals() {
    auto normals = make_unique<NormalArray>();
    // Initialize normals array with zero vectors
    normals->resize(m_width * m_height, glm::vec3(0.0f));
    
    // Calculate normals for each triangle and accumulate them at vertices
    for (int i = 0; i < m_width - 1; i++) {
      for (int j = 0; j < m_height - 1; j++) {
        int id1 = i * m_height + j;
        int id2 = i * m_height + j + 1;
        int id3 = (i + 1) * m_height + j;
        int id4 = (i + 1) * m_height + j + 1;
        
        // Calculate normals for first triangle (id1, id2, id3)
        glm::vec3 v1 = m_state.positions[id2] - m_state.positions[id1];
        glm::vec3 v2 = m_state.positions[id3] - m_state.positions[id1];
        glm::vec3 normal1 = glm::normalize(glm::cross(v1, v2));
        
        // Calculate normals for second triangle (id2, id4, id3)
        glm::vec3 v3 = m_state.positions[id4] - m_state.positions[id2];
        glm::vec3 v4 = m_state.positions[id3] - m_state.positions[id2];
        glm::vec3 normal2 = glm::normalize(glm::cross(v3, v4));
        
        // Accumulate normals at vertices
        (*normals)[id1] += normal1;
        (*normals)[id2] += normal1 + normal2;
        (*normals)[id3] += normal1 + normal2;
        (*normals)[id4] += normal2;
      }
    }
    
    // Normalize all vertex normals
    for (auto& normal : *normals) {
      normal = glm::normalize(normal);
    }
    return normals;
  }
  void Reset() {
    m_state = m_system.GetInitialState();
    m_time = 0.0f;
    auto positions = make_unique<PositionArray>();
    auto normals = make_unique<NormalArray>();
    for (int i = 0; i < m_num_particles; i++) {
      positions->push_back(m_state.positions[i]);
      normals->push_back(glm::vec3(0.0f, 0.0f, 0.0f));
    }
    mesh->UpdatePositions(std::move(positions));
    mesh->UpdateNormals(std::move(normals));
  }
 private:
  float m_dt, m_time;
  bool gentle_wind;
  int m_width, m_height;
  ParticleState m_state;
  PendulumSystem m_system;
  std::shared_ptr<ShaderProgram> shader;
  std::shared_ptr<VertexObject> mesh;
  // store as polymorphic base class, not the concrete ForwardEuler type
  std::unique_ptr<IntegratorBase<PendulumSystem, ParticleState>> m_integrator;
  int m_num_particles;
  //store raw child pointers; ownership is in SceneNode::children_
  std::vector<SceneNode*> particle_nodes;
};

}  // namespace GLOO
