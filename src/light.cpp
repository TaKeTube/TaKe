#include "light.h"
#include "scene.h"

PointAndNormal sample_on_light_op::operator()(const PointLight &l) const {
    return {l.position, Vector3{0, 0, 0}};
}

PointAndNormal sample_on_light_op::operator()(const DiffuseAreaLight &l) const {
    return std::visit(sample_on_shape_op{rng}, scene.shapes.at(l.shape_id));
}