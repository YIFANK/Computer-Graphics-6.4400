#pragma once
#include "gloo/SceneNode.hpp"
#include "ForwardEulerIntegrator.hpp"
#include "RK4Integrator.hpp"
#include "TrapezoidalIntegrator.hpp"
#include "IntegratorFactory.hpp"
#include "PendulumSystem.hpp"
#include "gloo/shaders/SimpleShader.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/RenderingComponent.hpp"

namespace GLOO {

class PendulumSystemNode : public SceneNode {
 public:
  PendulumSystemNode(IntegratorType integrator_type, float dt,int num_particles)
      : m_dt(dt), m_time(0.0f), m_num_particles(num_particles) {
    //initialize the pendulum system
    std::cerr << "Initializing pendulum system" << std::endl;
    for (int i = 0; i < m_num_particles; i++) {
      m_system.AddParticle(glm::vec3(-i * 0.5f, -i * 0.5f, 0.0f), 1.0f);
    }
    for (int i = 0; i < m_num_particles - 1; i++) {
      m_system.AddSpring(i, i + 1, 1000.0f, 0.707f);
    }
    //fix the first particle
    m_system.FixParticle(0);
    
    // Get initial state from the system
    m_state = m_system.GetInitialState();
    std::cerr << "Got initial state with " << m_state.positions.size() << " particles" << std::endl;

    // create integrator
    m_integrator = IntegratorFactory::CreateIntegrator<PendulumSystem, ParticleState>(
        integrator_type);
    std::cerr << "Creating integrator" << std::endl;
    
    shader = std::make_shared<SimpleShader>();
    mesh = PrimitiveFactory::CreateSphere(0.02f, 25, 25);
    for (int i = 0; i < m_num_particles; i++) {
      auto child = make_unique<SceneNode>();
      child->CreateComponent<ShadingComponent>(shader);
      child->CreateComponent<RenderingComponent>(mesh);
      child->GetTransform().SetPosition(m_state.positions[i]);
      SceneNode* child_ptr = child.get();
      AddChild(std::move(child));
      particle_nodes.push_back(child_ptr);
    }
    std::cerr << "Initialized pendulum system done" << std::endl;
  }

  void Update(double delta_time) override {
    int steps = static_cast<int>(delta_time / static_cast<double>(m_dt));
    for (int i = 0; i < steps; ++i) {
      m_state = m_integrator->Integrate(m_system, m_state, m_time, m_dt);
      m_time += m_dt;
    }
    for (int i = 0; i < m_num_particles; i++) {
      particle_nodes[i]->GetTransform().SetPosition(m_state.positions[i]);
    }
  }

 private:
  float m_dt, m_time;
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
