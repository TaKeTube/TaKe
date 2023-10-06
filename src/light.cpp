#include "light.h"
#include "scene.h"
#include <algorithm>

struct light_power_op {
    Real operator()(const PointLight &l) const;
    Real operator()(const AreaLight &l) const;
    Real operator()(const Envmap &l) const;

    const Scene &scene;
};

struct sample_on_light_op {
    PointAndNormal operator()(const PointLight &l) const;
    PointAndNormal operator()(const AreaLight &l) const;
    PointAndNormal operator()(const Envmap &l) const;

    const Scene &scene;
    const Vector3 &ref_pos;
    std::mt19937& rng;
};

struct sample_on_light_pdf_op {
    Real operator()(const PointLight &l) const;
    Real operator()(const AreaLight &l) const;
    Real operator()(const Envmap &l) const;

    const Scene &scene;
    const PointAndNormal &light_point;
    const Vector3 &ref_pos;
};

struct emission_op {
    Vector3 operator()(const PointLight &l) const;
    Vector3 operator()(const AreaLight &l) const;
    Vector3 operator()(const Envmap &l) const;

    const Scene &scene;
    const Vector3 &view_dir;
    const PointAndNormal &light_point;
};

struct init_sample_dist_op {
    void operator()(PointLight &l) const;
    void operator()(AreaLight &l) const;
    void operator()(Envmap &l) const;

    const Scene &scene;
};

#include "lights/point_light.inl"
#include "lights/area_light.inl"
#include "lights/envmap.inl"

Real get_light_power(const Scene &scene, const Light &l) {
    return std::visit(light_power_op{scene}, l);
}

PointAndNormal sample_on_light(const Scene &scene, 
                               const Light& l, 
                               const Vector3 &ref_pos, 
                               std::mt19937& rng) {
    return std::visit(sample_on_light_op{scene, ref_pos, rng}, l);
}

Real get_light_pdf(const Scene &scene, 
                   const Light& l,
                   const PointAndNormal &light_point,
                   const Vector3 &ref_pos) {
    return std::visit(sample_on_light_pdf_op{scene, light_point, ref_pos}, l);
}

Vector3 get_light_emission(const Scene &scene, 
                           const Light& l,
                           const Vector3 &view_dir, 
                           const PointAndNormal &light_point) {
    return std::visit(emission_op{scene, view_dir, light_point}, l);
}

void init_sample_dist(Light &l, const Scene &scene) {
    return std::visit(init_sample_dist_op{scene}, l);
}

int sample_light(const Scene &scene, std::mt19937& rng) {
    return static_cast<int>(floor(random_real(rng) * scene.lights.size()));
}

int sample_light_power(const Scene &scene, std::mt19937& rng) {
    const std::vector<Real> &power_cdf = scene.lights_power_cdf;
    Real u = random_real(rng);
    int size = static_cast<int>(power_cdf.size()) - 1;
    assert(size > 0);
    const Real *ptr = std::upper_bound(power_cdf.data(), power_cdf.data() + size + 1, u);
    int offset = std::clamp(int(ptr - power_cdf.data() - 1), 0, size - 1);
    return offset;
}

Real get_light_pmf(const Scene &scene, int id) {
    const std::vector<Real> &pmf = scene.lights_power_pmf;
    assert(id >= 0 && id < (int)pmf.size());
    return pmf[id];
}