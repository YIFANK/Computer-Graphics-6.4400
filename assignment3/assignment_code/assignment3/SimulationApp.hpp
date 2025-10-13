#ifndef SIMULATION_APP_H_
#define SIMULATION_APP_H_

#include "gloo/Application.hpp"
#include "ParticleSystemBase.hpp"
#include "IntegratorBase.hpp"
#include "ParticleState.hpp"
#include "IntegratorType.hpp"
#include "SimpleSystem.hpp"
#include "PendulumSystem.hpp"

namespace GLOO {
class SimulationApp : public Application {
 public:
  SimulationApp(const std::string& app_name,
                glm::ivec2 window_size,
                IntegratorType integrator_type,
                float integration_step);
  void SetupScene() override;

 private:
  IntegratorType integrator_type_;
  float integration_step_;
  std::string app_name_;
};
}  // namespace GLOO

#endif
