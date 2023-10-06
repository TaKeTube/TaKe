#pragma once
#include <variant>
#include "vector.h"
#include "matrix.h"
#include "transform.h"
#include "intersection.h"
#include "shape.h"
#include "distribution.h"
#include "texture.h"

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
    Texture intensity;
    Matrix4x4 to_world, to_local;
    Real scale;

    Distribution2D dist; // importance sampling luminance
};

using Light = std::variant<PointLight, AreaLight, Envmap>;

Real get_light_power(const Scene &scene, const Light &l);
PointAndNormal sample_on_light(const Scene &scene, 
                               const Light& l, 
                               const Vector3 &ref_pos, 
                               std::mt19937& rng);
Real get_light_pdf(const Scene &scene, 
                   const Light& l,
                   const PointAndNormal &light_point,
                   const Vector3 &ref_pos);
Vector3 get_light_emission(const Scene &scene, 
                           const Light& l, 
                           const Vector3 &view_dir, 
                           const PointAndNormal &light_point);
void init_sample_dist(Light &l, const Scene &scene);

int sample_light(const Scene &scene, std::mt19937& rng);
int sample_light_power(const Scene &scene, std::mt19937& rng);
Real get_light_pmf(const Scene &scene, int id);