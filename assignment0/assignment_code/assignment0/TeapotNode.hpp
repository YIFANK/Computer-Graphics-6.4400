#ifndef TEAPOT_NODE_H_
#define TEAPOT_NODE_H_
#include "gloo/SceneNode.hpp"
namespace GLOO {
class TeapotNode : public SceneNode {
public:
TeapotNode();
void ToggleColor();
void ChangeColor();
void SpinModel();
void Update(double delta_time) override;
};
}
#endif