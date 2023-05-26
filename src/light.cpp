#include "light.h"
#include "scene.h"

int sample_light(const Scene &scene, std::mt19937& rng) {
    return floor(random_double(rng) * scene.lights.size());
}

Real get_light_pdf(const Scene &scene, int light_id) {
    if(auto* l = std::get_if<DiffuseAreaLight>(&scene.lights[light_id])){
        auto shape = scene.shapes[l->shape_id];
        if(auto* s = std::get_if<Triangle>(&shape)){
            return 1/get_area(*s);
        }else if(auto* s = std::get_if<Sphere>(&shape)){
            // std::cout << 2/get_area(*s) << std::endl;
            return 1/get_area(*s);
        }
    }
    std::cout << light_id << std::endl;
    return 0;
}

PointAndNormal sample_on_light_op::operator()(const PointLight &l) const {
    return {l.position, Vector3{0, 0, 0}};
}

PointAndNormal sample_on_light_op::operator()(const DiffuseAreaLight &l) const {
    return std::visit(sample_on_shape_op{rng}, scene.shapes.at(l.shape_id));
}