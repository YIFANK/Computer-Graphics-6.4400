#ifndef POINT_LIGHT_NODE_H_
#define POINT_LIGHT_NODE_H_
#include "gloo/SceneNode.hpp"
namespace GLOO {
class PointLightNode : public SceneNode {
public:
PointLightNode();
void Update(double delta_time) override;
};
}
#endif