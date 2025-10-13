#pragma once
#include "ParticleSystemBase.hpp"
#include <glm/glm.hpp>

namespace GLOO {
class PendulumSystem : public ParticleSystemBase {
public:
    struct Spring {
        int id1;
        int id2;
        float k;
        float l0;
    };
    ParticleState ComputeTimeDerivative(const ParticleState& state, float time) const override {
        //gravity force
        glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0f);
        ParticleState derivative;
        int n = state.positions.size();
        derivative.positions.resize(n);
        derivative.velocities.resize(n);
        
        for (int i = 0; i < n; i++) {
            derivative.positions[i] = state.velocities[i];
            derivative.velocities[i] = gravity;
        }
        //spring force
        for(auto spring : springs) {
            glm::vec3 r1 = state.positions[spring.id1] - state.positions[spring.id2];
            float r = glm::length(r1);
            if (r > 0.0f) {
                glm::vec3 f = -spring.k * (r - spring.l0) * glm::normalize(r1);
                derivative.velocities[spring.id1] += f / masses[spring.id1];
                derivative.velocities[spring.id2] -= f / masses[spring.id2];
            }
        }
        //viscous force
        for(int i = 0; i < n; i++) {
            derivative.velocities[i] -= 0.1f * state.velocities[i] / masses[i];
        }
        for (int i = 0; i < n; i++) {
            if (fixed[i]) {
                derivative.positions[i] = glm::vec3(0.0f);
                derivative.velocities[i] = glm::vec3(0.0f);
            }
        }   
        return derivative;
    }
    void AddParticle(const glm::vec3& position, const float& mass) {
        positions.push_back(position);
        velocities.push_back(glm::vec3(0.0f));
        masses.push_back(mass);
        fixed.push_back(false);
    }
    void AddSpring(const int& id1, const int& id2, const float& k, const float& l0) {
        springs.push_back(Spring{id1, id2, k, l0});
    }
    void FixParticle(const int& id) {
        fixed[id] = true;
    }
    ParticleState GetInitialState() const {
        ParticleState state;
        state.positions = positions;
        state.velocities = velocities;
        return state;
    }
private:
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> velocities;
    std::vector<float> masses;
    std::vector<Spring> springs;
    std::vector<int> fixed;
};
}
