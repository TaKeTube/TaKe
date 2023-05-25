#pragma once
#include <optional>
#include "vector.h"
#include "intersection.h"
#include "texture.h"

struct Diffuse {
    Texture reflectance;
};

struct Mirror {
    Texture reflectance;
    Real eta = 1;
};

struct Plastic {
    Texture reflectance;
    Real eta = 1;
};

struct Phong {
    Texture reflectance; // Ks
    Real exponent; // alpha
};

struct BlinnPhong {
    Texture reflectance; // Ks
    Real exponent; // alpha
};

struct BlinnPhongMicrofacet {
    Texture reflectance; // Ks
    Real exponent; // alpha
};

using Material = std::variant<Diffuse,
                              Mirror,
                              Plastic,
                              Phong,
                              BlinnPhong,
                              BlinnPhongMicrofacet>;

struct SampleRecord {
    Vector3 dir_out;
    Real pdf;
};

Vector3 eval(
    const Material &material,
    const Vector3 &dir_in,
    const SampleRecord &record,
    const Intersection &v,
    const TexturePool &pool);

std::optional<SampleRecord> sample_bsdf(
    const Material &material,
    const Vector3 &dir_in,
    const Intersection &v,
    const TexturePool &pool,
    std::mt19937 &rng);

inline Vector3 sample_hemisphere_cos(std::mt19937& rng) {
    Real u1 = random_double(rng);
    Real u2 = random_double(rng);
    
    Real phi = c_TWOPI * u2;
    Real sqrt_u1 = sqrt(std::clamp(u1, Real(0), Real(1)));
    return Vector3{
        cos(phi) * sqrt_u1, 
        sin(phi) * sqrt_u1,
        sqrt(std::clamp(1 - u1, Real(0), Real(1)))
    };
}