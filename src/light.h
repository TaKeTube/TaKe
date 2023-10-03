#pragma once
#include <variant>
#include "vector.h"
#include "matrix.h"
#include "intersection.h"
#include "shape.h"
#include "distribution.h"

struct Scene;

struct PointLight {
    Vector3 intensity;
    Vector3 position;    
};

struct AreaLight {
    int shape_id;
    Vector3 intensity;
};

struct Envmap {
    Texture values;
    Matrix4x4 to_world, to_local;
    Real scale;

    Distribution2D dist; // importance sampling luminance
};

using Light = std::variant<PointLight, AreaLight, Envmap>;

Real light_power(const Scene &scene, const Light &light);
PointAndNormal sample_on_light(const Scene &scene, 
                               const Light& l, 
                               const Vector3 &ref_pos, 
                               std::mt19937& rng);
Real get_light_pdf(const Scene &scene,
                   const Light &light,
                   const PointAndNormal &light_point,
                   const Vector3 &ref_pos
);
Vector3 emission(const Light &light);

int sample_light(const Scene &scene, std::mt19937& rng);
int sample_light_power(const Scene &scene, std::mt19937& rng);
Real get_light_pmf(const Scene &scene, int id);