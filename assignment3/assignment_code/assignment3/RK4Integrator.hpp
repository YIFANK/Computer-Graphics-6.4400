#ifndef RK4_INTEGRATOR_H_
#define RK4_INTEGRATOR_H_

#include "IntegratorBase.hpp"

namespace GLOO {
template <class TSystem, class TState>
class RK4Integrator : public IntegratorBase<TSystem, TState> {
  TState Integrate(const TSystem& system,
                   const TState& state,
                   float start_time,
                   float dt) const override {
    auto f0 = system.ComputeTimeDerivative(state, start_time);
    auto f1 = system.ComputeTimeDerivative(state + (dt / 2) * f0, start_time + dt / 2);
    auto f2 = system.ComputeTimeDerivative(state + (dt / 2) * f1, start_time + dt / 2);
    auto f3 = system.ComputeTimeDerivative(state + dt * f2, start_time + dt);
    return state + dt / 6 * (f0 + 2 * f1 + 2 * f2 + f3);
  }
};
}  // namespace GLOO

#endif
