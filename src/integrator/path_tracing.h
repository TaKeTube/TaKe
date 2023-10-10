#include "scene.h"

// Path tracing with multi-sample version of MIS
// we deterministically shooting rays for both lights and BRDFs and weighing them.
Vector3 path_tracing(const Scene& scene, const Ray& ray, std::mt19937& rng){
    Ray r = ray;
    std::optional<Intersection> v_ = scene_intersect(scene, r);
    if(!v_) {
        if (has_envmap(scene)) {
            const Light &envmap = get_envmap(scene);
            return get_light_emission(scene, envmap, -ray.dir, PointAndNormal{Vector3{0, 0, 0}, -ray.dir});
        }
        return scene.background_color;
    }
    Intersection v = *v_;

    Vector3 radiance   = {Real(0), Real(0), Real(0)};
    Vector3 throughput = {Real(1), Real(1), Real(1)};

    if(v.area_light_id != -1) {
        const Light& light = scene.lights.at(v.area_light_id);
        if (auto* l = std::get_if<AreaLight>(&light))
            radiance += throughput * get_light_emission(scene, light, -r.dir, PointAndNormal{v.pos, v.geo_normal});
    }

    for(int i = 0; i <= scene.options.max_depth; ++i){

        Vector3 dir_in    = -r.dir;
        const Material& m = scene.materials[v.material_id];
        
        // Sampling Light
        Vector3 C1 = Vector3{Real(0), Real(0), Real(0)};
        if(scene.lights.size() > 0 && !is_specular(m)){
            int light_id                = sample_light(scene, rng);
            const Light& light          = scene.lights[light_id];
            PointAndNormal& light_point = sample_on_light(scene, light, v.pos, rng);
            auto& [light_pos, light_n]  = light_point;
            
            Real d;
            Vector3 light_dir;
            Real J_light = 1;
            if(is_envmap(light)){
                light_dir = -light_point.normal;
                d = infinity<Real>();
            } else {
                light_dir = normalize(light_pos - v.pos);
                d = length(light_pos - v.pos);
                J_light = (d * d) / (fmax(dot(-light_n, light_dir), Real(0)) * scene.lights.size());
            }

            Real light_pdf = get_light_pdf(scene, light, light_point, v.pos) * J_light;
            if(light_pdf <= 0){
                // std::cout << light_pdf << "light pdf break" << std::endl;
            }
            Real bsdf_pdf = get_bsdf_pdf(m, dir_in, light_dir, v, scene.textures);
            // Cannot break here because it is possible that the light is behind the current point
            // Also, seems the truncation above leads to infinity light pdf, inf/inf leads to NaN so exclude this case
            if(bsdf_pdf > 0 && light_pdf > 0 && !std::isinf(light_pdf)){
                SampleRecord record = {};
                record.dir_out      = light_dir;
                Vector3 FG          = eval(m, dir_in, record, v, scene.textures);

                // TODO Multiple lights may cause different behavior here,
                // for example, sample a light point that is blocked by another light
                // needs to be checked here
                Ray shadow_r = Ray{v.pos, light_dir, c_EPSILON, (1 - c_EPSILON) * d};
                if(!scene_occluded(scene, shadow_r)){
                    C1 = FG * get_light_emission(scene, light, -light_dir, light_point) * light_pdf / (light_pdf * light_pdf + bsdf_pdf * bsdf_pdf);
                }
            }
        }
        radiance += throughput * C1;

        // Sampling bsdf
        Vector3 C2 = Vector3{Real(0), Real(0), Real(0)};
        Vector3 n = dot(dir_in, v.shading_normal) < 0 ? -v.shading_normal : v.shading_normal;
        std::optional<SampleRecord> record_ = sample_bsdf(m, dir_in, v, scene.textures, rng);
        if(!record_){
            // std::cout << "record break" << std::endl;
            break;
        }
        SampleRecord& record = *record_;
        Vector3 FG           = eval(m, dir_in, record, v, scene.textures);
        Vector3 dir_out      = normalize(record.dir_out);
        Real bsdf_pdf        = record.pdf;
        if(bsdf_pdf <= 0){
            // std::cout << "pdf break" << std::endl;
            break;
        }
        r = Ray{v.pos, dir_out, c_EPSILON, infinity<Real>()};
        std::optional<Intersection> new_v_ = scene_intersect(scene, r);

        // We hit nothing
        if(!new_v_){
            // std::cout << "bg break" << std::endl;
            if(has_envmap(scene)){
                Vector3 light_dir           = dir_out;
                const Light &light          = get_envmap(scene);
                PointAndNormal light_point  = {Vector3{0, 0, 0}, -light_dir};

                Real light_pdf         = get_light_pdf(scene, light, light_point, v.pos);

                Vector3 light_emission = get_light_emission(scene, light, -light_dir, light_point);
                Real w_over_pdf        = is_specular(m) ? (1 / bsdf_pdf): (bsdf_pdf / (light_pdf * light_pdf + bsdf_pdf * bsdf_pdf));
                if(light_pdf > 0){
                    C2 = FG * light_emission * w_over_pdf;
                }
            }else{
                C2 = FG * scene.background_color / bsdf_pdf;
            }
            radiance += throughput * C2;
            break;
        }
        // We hit an area light
        if(new_v_->area_light_id != -1){
            Vector3 &light_pos          = new_v_->pos;
            Vector3 light_dir           = normalize(light_pos - v.pos);
            const Light& light          = scene.lights[new_v_->area_light_id];
            PointAndNormal light_point  = {new_v_->pos, new_v_->geo_normal};

            Real d         = length(light_pos - v.pos);
            Real J_light   = (d * d) / (fmax(dot(-new_v_->geo_normal, light_dir), Real(0)) * scene.lights.size());
            Real light_pdf = get_light_pdf(scene, light, light_point, v.pos) * J_light;
            
            Vector3 light_emission = get_light_emission(scene, light, -light_dir, light_point);
            Real w_over_pdf        = is_specular(m) ? (1 / bsdf_pdf): (bsdf_pdf / (light_pdf * light_pdf + bsdf_pdf * bsdf_pdf));
            if(light_pdf > 0){
                C2 = FG * light_emission * w_over_pdf;
            }else{
                // std::cout << dot(-new_v_->geo_normal, light_dir) << std::endl;
                break;
            }
        }
        radiance += throughput * C2;
        
        // This line would lead to a biased result:
        // throughput *= FG / (is_specular(m) ? bsdf_pdf : (light_pdf + bsdf_pdf));
        // A complicated proof shows this line combined with terminated NEE leads to the unbiased result:
        throughput *= FG / bsdf_pdf;
        v = *new_v_;
    }
    return radiance;
}

// Path tracing without MIS
Vector3 path_tracing_raw(const Scene& scene, const Ray& ray, std::mt19937& rng){
    Ray r = ray;
    std::optional<Intersection> v_ = scene_intersect(scene, r);
    if(!v_) return scene.background_color;
    Intersection v = *v_;

    Vector3 radiance = {Real(0), Real(0), Real(0)};
    Vector3 throughput = {Real(1), Real(1), Real(1)};
    for(int i = 0; i <= scene.options.max_depth; ++i){
        if(v.area_light_id != -1) {
            const Light& light = scene.lights.at(v.area_light_id);
            if (auto* l = std::get_if<AreaLight>(&light)){
                radiance += throughput * get_light_emission(scene, light, -r.dir, PointAndNormal{v.pos, v.geo_normal});
                break;
            }
        } else {
            Vector3 dir_in = -r.dir;
            Vector3 n = dot(dir_in, v.shading_normal) < 0 ? -v.shading_normal : v.shading_normal;
            std::optional<SampleRecord> record_ = sample_bsdf(scene.materials[v.material_id], dir_in, v, scene.textures, rng);
            if(!record_){
                // std::cout << "record break" << std::endl;
                break;
            }
            SampleRecord& record = *record_;
            Vector3 FG = eval(scene.materials[v.material_id], dir_in, record, v, scene.textures);
            Vector3 dir_out = normalize(record.dir_out);
            Real pdf = record.pdf;
            if(pdf <= Real(0)){
                // std::cout << "pdf break" << std::endl;
                break;
            }
            throughput *= FG / pdf;
            r = Ray{v.pos, dir_out, c_EPSILON, infinity<Real>()};
            std::optional<Intersection> v_ = scene_intersect(scene, r);
            if(!v_){
                // std::cout << "bg break" << std::endl;
                radiance += throughput * scene.background_color;
                break;
            }
            v = *v_;
        }
    }
    return radiance;
}

// Path tracing with one-sample variant of MIS
// instead of deterministically shooting rays for both lights and BRDFs and weighing them, we randomly choose one and combine the distribution.
Vector3 path_tracing_one_sample_MIS(const Scene& scene, const Ray& ray, std::mt19937& rng){
    Ray r = ray;
    std::optional<Intersection> v_ = scene_intersect(scene, r);
    if(!v_) return scene.background_color;
    Intersection v = *v_;

    Vector3 radiance = {Real(0), Real(0), Real(0)};
    Vector3 throughput = {Real(1), Real(1), Real(1)};
    for(int i = 0; i <= scene.options.max_depth; ++i){
        if(v.area_light_id != -1) {
            const Light& light = scene.lights.at(v.area_light_id);
            if (auto* l = std::get_if<AreaLight>(&light)){
                // std::cout << throughput << std::endl;
                radiance += throughput * get_light_emission(scene, light, -r.dir, PointAndNormal{v.pos, v.geo_normal});
                break;
            }
        }

        Vector3 dir_in = -r.dir;
        const Material& m = scene.materials[v.material_id];
        bool is_specular = false;
        if(std::holds_alternative<Plastic>(m) || std::holds_alternative<Mirror>(m))
            is_specular = true;
        
        // For mirror, sampling light has no meaning because only the perfect reflection angle will produce non-zero FG
        // If we do NEE it will be inefficient
        if(scene.lights.size() > 0 && !is_specular && random_real(rng) <= 0.5){
            // Sampling Light
            int light_id = sample_light(scene, rng);
            auto light = scene.lights[light_id];
            if (auto* l = std::get_if<AreaLight>(&light)) {
                auto& light_point = sample_on_light(scene, *l, v.pos, rng);
                auto& [light_pos, light_n] = light_point;
                Real d = length(light_pos - v.pos);
                Vector3 light_dir = normalize(light_pos - v.pos);

                Real light_pdf = get_light_pdf(scene, light, light_point, v.pos) * (d * d) / (fmax(dot(-light_n, light_dir), Real(0)) * scene.lights.size());
                if(light_pdf <= 0){
                    // std::cout << light_pdf << "light pdf break" << std::endl;
                    break;
                }
                Real bsdf_pdf = get_bsdf_pdf(m, dir_in, light_dir, v, scene.textures);
                if(bsdf_pdf <= 0){
                    // std::cout << "bsdf pdf break" << std::endl;
                    break;
                }
                
                SampleRecord record = {};
                record.dir_out = light_dir;
                Vector3 FG = eval(m, dir_in, record, v, scene.textures);

                r = Ray{v.pos, light_dir, c_EPSILON, infinity<Real>()};
                std::optional<Intersection> v_ = scene_intersect(scene, r);
                // No need for vertex check cause we will always hit the light or an obstacle
                // if(!v_){
                //     // std::cout << "bg break" << std::endl;
                //     radiance += throughput * scene.background_color;
                //     break;
                // }
                v = *v_;
                // Seems no need for this
                // if(v.area_light_id == -1){
                //     break;
                // }
                throughput *= FG / (Real(0.5) * light_pdf + Real(0.5) * bsdf_pdf);
                // at the beginning of the next loop, we will break
            }
        }else{
            // Sampling bsdf
            Vector3 n = dot(dir_in, v.shading_normal) < 0 ? -v.shading_normal : v.shading_normal;
            std::optional<SampleRecord> record_ = sample_bsdf(m, dir_in, v, scene.textures, rng);
            if(!record_){
                // std::cout << "record break" << std::endl;
                break;
            }
            SampleRecord& record = *record_;
            Vector3 FG = eval(m, dir_in, record, v, scene.textures);
            Vector3 dir_out = normalize(record.dir_out);
            Real bsdf_pdf = record.pdf;
            if(bsdf_pdf <= Real(0)){
                // std::cout << "pdf break" << std::endl;
                break;
            }
            r = Ray{v.pos, dir_out, c_EPSILON, infinity<Real>()};
            std::optional<Intersection> new_v_ = scene_intersect(scene, r);

            Real pdf = (scene.lights.empty() || is_specular) ? bsdf_pdf : Real(0.5) * bsdf_pdf;

            if(!new_v_){
                // std::cout << "bg break" << std::endl;
                throughput *= FG / pdf;
                radiance += throughput * scene.background_color;
                break;
            }
            if(!is_specular && new_v_->area_light_id != -1){
                Vector3 &light_pos = new_v_->pos;
                Real d = length(light_pos - v.pos);
                Vector3 light_dir = normalize(light_pos - v.pos);
                Real light_pdf = get_light_pdf(scene, scene.lights[new_v_->area_light_id], {new_v_->pos, new_v_->geo_normal}, v.pos) * (d * d) / (fmax(dot(-new_v_->geo_normal, light_dir), Real(0)) * scene.lights.size());
                if(light_pdf <= 0){
                    // std::cout << dot(-new_v_->geo_normal, light_dir) << std::endl;
                    break;
                }
                pdf += Real(0.5) * light_pdf;
            }
            throughput *= FG / pdf;
            v = *new_v_;
        }
    }
    return radiance;
}

// Path tracing with one-sample variant of MIS with light picking by power (Seems to have bugs)
Vector3 path_tracing_one_sample_MIS_power(const Scene& scene, const Ray& ray, std::mt19937& rng){
    Ray r = ray;
    std::optional<Intersection> v_ = scene_intersect(scene, r);
    if(!v_) return scene.background_color;
    Intersection v = *v_;

    Real eta_scale = Real(1);
    Vector3 radiance = {Real(0), Real(0), Real(0)};
    Vector3 throughput = {Real(1), Real(1), Real(1)};
    for(int i = 0; i <= scene.options.max_depth; ++i){
        if(v.area_light_id != -1) {
            const Light& light = scene.lights.at(v.area_light_id);
            if (auto* l = std::get_if<AreaLight>(&light)){
                // std::cout << throughput << std::endl;
                radiance += throughput * get_light_emission(scene, light, -r.dir, PointAndNormal{v.pos, v.geo_normal});
                break;
            }
        }
        
        Vector3 dir_in = -r.dir;
        const Material& m = scene.materials[v.material_id];
        bool is_specular = false;
        if(std::holds_alternative<Plastic>(m) || std::holds_alternative<Mirror>(m))
            is_specular = true;
        
        if(scene.lights.size() > 0 && !is_specular && random_real(rng) <= 0.5){
            // Sampling Light
            int light_id = sample_light_power(scene, rng);
            auto light = scene.lights[light_id];
            if (auto* l = std::get_if<AreaLight>(&light)) {
                auto& light_point = sample_on_light(scene, *l, v.pos, rng);
                auto& [light_pos, light_n] = light_point;
                Real d = length(light_pos - v.pos);
                Vector3 light_dir = normalize(light_pos - v.pos);

                Real light_pdf = get_light_pdf(scene, light, light_point, v.pos) * (d * d) * get_light_pmf(scene, light_id) / (fmax(dot(-light_n, light_dir), Real(0)));
                if(light_pdf <= 0){
                    // std::cout << light_pdf << "light pdf break" << std::endl;
                    break;
                }
                Real bsdf_pdf = get_bsdf_pdf(m, dir_in, light_dir, v, scene.textures);
                if(bsdf_pdf <= 0){
                    // std::cout << "bsdf pdf break" << std::endl;
                    break;
                }
                
                SampleRecord record = {};
                record.dir_out = light_dir;
                Vector3 FG = eval(m, dir_in, record, v, scene.textures);

                r = Ray{v.pos, light_dir, c_EPSILON, infinity<Real>()};
                std::optional<Intersection> v_ = scene_intersect(scene, r);
                if(!v_){
                    // std::cout << "bg break" << std::endl;
                    radiance += throughput * scene.background_color;
                    break;
                }
                v = *v_;
                if(v.area_light_id == -1){
                    break;
                }
                throughput *= FG / (Real(0.5) * light_pdf + Real(0.5) * bsdf_pdf);
            }
        }else{
            // Sampling bsdf
            Vector3 n = dot(dir_in, v.shading_normal) < 0 ? -v.shading_normal : v.shading_normal;
            std::optional<SampleRecord> record_ = sample_bsdf(m, dir_in, v, scene.textures, rng);
            if(!record_){
                // std::cout << "record break" << std::endl;
                break;
            }
            SampleRecord& record = *record_;
            Vector3 FG = eval(m, dir_in, record, v, scene.textures);
            Vector3 dir_out = normalize(record.dir_out);
            Real bsdf_pdf = record.pdf;
            if(bsdf_pdf <= Real(0)){
                // std::cout << "pdf break" << std::endl;
                break;
            }
            r = Ray{v.pos, dir_out, c_EPSILON, infinity<Real>()};
            std::optional<Intersection> new_v_ = scene_intersect(scene, r);

            Real pdf = (scene.lights.empty() || is_specular) ? bsdf_pdf : Real(0.5) * bsdf_pdf;

            if(!new_v_){
                // std::cout << "bg break" << std::endl;
                throughput *= FG / pdf;
                radiance += throughput * scene.background_color;
                break;
            }
            if(!is_specular && new_v_->area_light_id != -1){
                Vector3 &light_pos = new_v_->pos;
                Real d = length(light_pos - v.pos);
                Vector3 light_dir = normalize(light_pos - v.pos);
                Real light_pdf = get_light_pdf(scene, scene.lights[new_v_->area_light_id], {new_v_->pos, new_v_->geo_normal}, v.pos) * (d * d) * get_light_pmf(scene, new_v_->area_light_id) / fmax(dot(-new_v_->geo_normal, light_dir), Real(0));
                if(light_pdf <= 0){
                    // std::cout << dot(-new_v_->geo_normal, light_dir) << std::endl;
                    break;
                }
                pdf += Real(0.5) * light_pdf;
            }
            throughput *= FG / pdf;
            v = *new_v_;
        }
    }
    return radiance;
}
