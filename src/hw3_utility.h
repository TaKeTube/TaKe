#pragma once
#include "scene.h"

namespace hw3 {

    Vector3 hw3::trace_ray_without_sample(const Scene& scene, const Ray& r){
        std::optional<Intersection> v_ = scene_intersect(scene, r);
        if(!v_) return scene.background_color;
        Intersection v = *v_;
        Vector3 n = dot(r.dir, v.shading_normal) > 0 ? -v.shading_normal : v.shading_normal;

        if(scene.materials[v.material_id].type == MaterialType::Diffuse){
            Vector3 color = {Real(0), Real(0), Real(0)};
            for(auto& light:scene.lights){
                PointLight l;
                if(auto* lp = std::get_if<PointLight>(&light)){
                    l = *lp;
                } else {
                    continue;
                }
                Real d = length(l.position - v.pos);
                Vector3 light_dir = normalize(l.position - v.pos);
                Ray shadow_ray = {v.pos, light_dir, c_EPSILON, (1 - c_EPSILON) * d};
                if(!scene_occluded(scene, shadow_ray)){
                    const Vector3& Kd = eval(scene.materials[v.material_id].reflectance, v.uv, scene.textures);
                    color += Kd * max(dot(n, light_dir), Real(0)) * l.intensity / (c_PI * d * d);
                }
            }
            return color;
        }else if(scene.materials[v.material_id].type == MaterialType::Mirror){
            Ray reflect_ray = {v.pos, r.dir - 2*dot(r.dir, n) * n, c_EPSILON, infinity<Real>()};
            const Vector3& F0 = eval(scene.materials[v.material_id].reflectance, v.uv, scene.textures);
            Vector3 F = F0 + (1 - F0) * pow(1 - dot(n, reflect_ray.dir), 5);
            return F * hw3::trace_ray_without_sample(scene, reflect_ray);
        }else if(scene.materials[v.material_id].type == MaterialType::Plastic){
            // Compute diffuse color
            Vector3 diffuse = {Real(0), Real(0), Real(0)};
            for(auto& light:scene.lights){
                PointLight l;
                if(auto* lp = std::get_if<PointLight>(&light)){
                    l = *lp;
                } else {
                    continue;
                }
                Real d = length(l.position - v.pos);
                Vector3 light_dir = normalize(l.position - v.pos);
                Ray shadow_ray = {v.pos, light_dir, c_EPSILON, (1 - c_EPSILON) * d};
                if(!scene_occluded(scene, shadow_ray)){
                    const Vector3& Kd = eval(scene.materials[v.material_id].reflectance, v.uv, scene.textures);
                    diffuse += Kd * max(dot(n, light_dir), Real(0)) * l.intensity / (c_PI * d * d);
                }
            }
            // Compute specular color
            Ray reflect_ray = {v.pos, r.dir - 2*dot(r.dir, n) * n, c_EPSILON, infinity<Real>()};
            Vector3 specular = hw3::trace_ray_without_sample(scene, reflect_ray);
            // Compute blend color
            Real eta = scene.materials[v.material_id].eta;
            Real F0 = pow((eta - 1)/(eta + 1), 2);
            Real F = F0 + (1 - F0) * pow(1 - dot(n, reflect_ray.dir), 5);
            return F * specular + (1 - F) * diffuse;
        }else{
            return scene.background_color;
        }
    }

    Vector3 hw3::trace_ray(const Scene& scene, const Ray& r, std::mt19937& rng){
        std::optional<Intersection> v_ = scene_intersect(scene, r);
        if(!v_) return scene.background_color;
        Intersection v = *v_;
        Vector3 n = dot(r.dir, v.shading_normal) > 0 ? -v.shading_normal : v.shading_normal;

        if(v.area_light_id != -1) {
            const Light& light = scene.lights.at(v.area_light_id);
            if (auto* l = std::get_if<DiffuseAreaLight>(&light))
                return l->intensity;
        } else if(scene.materials[v.material_id].type == MaterialType::Diffuse){
            Vector3 color = {Real(0), Real(0), Real(0)};
            for(auto& light:scene.lights){
                if (auto* l = std::get_if<PointLight>(&light)){
                    Real d = length(l->position - v.pos);
                    Vector3 light_dir = normalize(l->position - v.pos);
                    Ray shadow_ray = {v.pos, light_dir, c_EPSILON, (1 - c_EPSILON) * d};
                    if(!scene_occluded(scene, shadow_ray)){
                        const Vector3& Kd = eval(scene.materials[v.material_id].reflectance, v.uv, scene.textures);
                        color += Kd * max(dot(n, light_dir), Real(0)) * l->intensity / (c_PI * d * d);
                    }
                } else if (auto* l = std::get_if<DiffuseAreaLight>(&light)) {
                    auto& [light_pos, light_n] = sample_on_light(scene, *l, rng);
                    Real d = length(light_pos - v.pos);
                    Vector3 light_dir = normalize(light_pos - v.pos);
                    Ray shadow_ray = {v.pos, light_dir, c_EPSILON, (1 - c_EPSILON) * d};
                    if(!scene_occluded(scene, shadow_ray)){
                        Real pdf_reciprocal = get_area(scene.shapes.at(l->shape_id));
                        const Vector3& Kd = eval(scene.materials[v.material_id].reflectance, v.uv, scene.textures);
                        color += pdf_reciprocal * Kd * max(dot(n, light_dir), Real(0)) * l->intensity * max(dot(-light_n, light_dir), Real(0)) / (c_PI * d * d);
                    }
                }
            }
            return color;
        }else if(scene.materials[v.material_id].type == MaterialType::Mirror){
            Ray reflect_ray = {v.pos, r.dir - 2*dot(r.dir, n) * n, c_EPSILON, infinity<Real>()};
            const Vector3& F0 = eval(scene.materials[v.material_id].reflectance, v.uv, scene.textures);
            Vector3 F = F0 + (1 - F0) * pow(1 - dot(n, reflect_ray.dir), 5);
            return F * hw3::trace_ray(scene, reflect_ray, rng);
        }else if(scene.materials[v.material_id].type == MaterialType::Plastic){
            // Compute diffuse color
            Vector3 diffuse = {Real(0), Real(0), Real(0)};
            for(auto& light:scene.lights){
                if (auto* l = std::get_if<PointLight>(&light)){
                    Real d = length(l->position - v.pos);
                    Vector3 light_dir = normalize(l->position - v.pos);
                    Ray shadow_ray = {v.pos, light_dir, c_EPSILON, (1 - c_EPSILON) * d};
                    if(!scene_occluded(scene, shadow_ray)){
                        const Vector3& Kd = eval(scene.materials[v.material_id].reflectance, v.uv, scene.textures);
                        diffuse += Kd * max(dot(n, light_dir), Real(0)) * l->intensity / (c_PI * d * d);
                    }
                } else if (auto* l = std::get_if<DiffuseAreaLight>(&light)) {
                    auto& [light_pos, light_n] = sample_on_light(scene, *l, rng);
                    Real d = length(light_pos - v.pos);
                    Vector3 light_dir = normalize(light_pos - v.pos);
                    Ray shadow_ray = {v.pos, light_dir, c_EPSILON, (1 - c_EPSILON) * d};
                    if(!scene_occluded(scene, shadow_ray)){
                        Real pdf_reciprocal = get_area(scene.shapes.at(l->shape_id));
                        const Vector3& Kd = eval(scene.materials[v.material_id].reflectance, v.uv, scene.textures);
                        diffuse += pdf_reciprocal * Kd * max(dot(n, light_dir), Real(0)) * l->intensity * max(dot(-light_n, light_dir), Real(0)) / (c_PI * d * d);
                    }
                }
            }
            // Compute specular color
            Ray reflect_ray = {v.pos, r.dir - 2*dot(r.dir, n) * n, c_EPSILON, infinity<Real>()};
            Vector3 specular = hw3::trace_ray(scene, reflect_ray, rng);
            // Compute blend color
            Real eta = scene.materials[v.material_id].eta;
            Real F0 = pow((eta - 1)/(eta + 1), 2);
            Real F = F0 + (1 - F0) * pow(1 - dot(n, reflect_ray.dir), 5);
            return F * specular + (1 - F) * diffuse;
        }else{
            return scene.background_color;
        }
    }

}