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

struct DisneyDiffuse {
    Texture reflectance;
    Real roughness;
    Real subsurface;
};

struct DisneyMetal {
    Texture reflectance;
    Real roughness;
    Real anisotropic;
};

struct DisneyGlass {
    Texture reflectance;
    Real roughness;
    Real anisotropic;

    Real eta; // internal IOR / externalIOR
};

struct DisneyClearcoat {
    Real clearcoat_gloss;
};

struct DisneySheen {
    Texture reflectance;
    Real sheen_tint;
};

struct DisneyBSDF {
    Texture reflectance;
    Real specular_transmission;
    Real metallic;
    Real subsurface;
    Real specular;
    Real roughness;
    Real specular_tint;
    Real anisotropic;
    Real sheen;
    Real sheen_tint;
    Real clearcoat;
    Real clearcoat_gloss;

    Real eta;
};

using Material = std::variant<Diffuse,
                              Mirror,
                              Plastic,
                              Phong,
                              BlinnPhong,
                              BlinnPhongMicrofacet,
                              DisneyDiffuse,
                              DisneyMetal,
                              DisneyGlass,
                              DisneyClearcoat,
                              DisneySheen,
                              DisneyBSDF>;

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

Real get_bsdf_pdf(
    const Material &material,
    const Vector3 &dir_in,
    const Vector3 &dir_out,
    const Intersection &v,
    const TexturePool &pool);

inline Vector3 sample_hemisphere_cos(std::mt19937& rng) {
    Real u1 = random_real(rng);
    Real u2 = random_real(rng);
    
    Real phi = c_TWOPI * u2;
    Real sqrt_u1 = sqrt(std::clamp(u1, Real(0), Real(1)));
    return Vector3{
        cos(phi) * sqrt_u1, 
        sin(phi) * sqrt_u1,
        sqrt(std::clamp(1 - u1, Real(0), Real(1)))
    };
}

inline Real compute_blinn_phong_G_hat(Vector3 omega, Vector3 n, Real alpha) {
    Real odn = dot(omega, n);
    Real a = sqrt(Real(0.5) * alpha + 1)/sqrt(1/(odn * odn) - 1);
    Real a2 = a*a;
    Real G_hat = a < Real(1.6) ? (Real(3.535)*a+Real(2.181)*a2)/(1+Real(2.276)*a+Real(2.577)*a2) : 1;
    return G_hat;
}