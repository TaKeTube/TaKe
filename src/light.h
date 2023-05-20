#pragma once
#include "vector.h"
#include "intersection.h"
#include "shape.h"

struct Scene;

struct PointLight {
    Vector3 intensity;
    Vector3 position;    
};

struct DiffuseAreaLight {
    int shape_id;
    Vector3 intensity;
};

using Light = std::variant<PointLight, DiffuseAreaLight>;

struct sample_on_light_op {
    PointAndNormal operator()(const PointLight &l) const;
    PointAndNormal operator()(const DiffuseAreaLight &l) const;

    const Scene &scene;
    std::mt19937& rng;
};

inline PointAndNormal sample_on_light(const Scene &scene, const Light& l, std::mt19937& rng) {
    return std::visit(sample_on_light_op{scene, rng}, l);
}