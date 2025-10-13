#pragma once
#include "gloo/SceneNode.hpp"
#include "ForwardEulerIntegrator.hpp"
#include "RK4Integrator.hpp"
#include "TrapezoidalIntegrator.hpp"
#include "IntegratorFactory.hpp"
#include "SimpleSystem.hpp"
#include "gloo/shaders/SimpleShader.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/RenderingComponent.hpp"

namespace GLOO {

class SimpleSystemNode : public SceneNode {
 public:
  SimpleSystemNode(IntegratorType integrator_type, float dt)
      : m_dt(dt), m_time(0.0f) {
    // initialize particle state
    m_state.positions.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
    m_state.velocities.push_back(glm::vec3(0.0f));

    // create integrator (Forward Euler)
    m_integrator = IntegratorFactory::CreateIntegrator<SimpleSystem, ParticleState>(
        integrator_type);
    shader = std::make_shared<SimpleShader>();
    mesh = PrimitiveFactory::CreateSphere(0.02f, 25, 25);
    CreateComponent<ShadingComponent>(shader);
    CreateComponent<RenderingComponent>(mesh);
  }

  void Update(double delta_time) override {
    int steps = static_cast<int>(delta_time / static_cast<double>(m_dt));
    for (int i = 0; i < steps; ++i) {
      m_state = m_integrator->Integrate(m_system, m_state, m_time, m_dt);
      m_time += m_dt;
    }
    GetTransform().SetPosition(m_state.positions[0]);
  }

 private:
  float m_dt, m_time;
  ParticleState m_state;
  SimpleSystem m_system;
  std::shared_ptr<ShaderProgram> shader;
  std::shared_ptr<VertexObject> mesh;
  // store as polymorphic base class, not the concrete ForwardEuler type
  std::unique_ptr<IntegratorBase<SimpleSystem, ParticleState>> m_integrator;
};

}  // namespace GLOO
