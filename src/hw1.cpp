#include "hw1.h"
#include "hw1_scenes.h"

using namespace hw1;

Image3 hw_1_1(const std::vector<std::string> &/*params*/) {
    // Homework 1.1: generate camera rays and output the ray directions
    // The camera is positioned at (0, 0, 0), facing towards (0, 0, -1),
    // with an up vector (0, 1, 0) and a vertical field of view of 90 degree.

    Image3 img(640 /* width */, 480 /* height */);

    Real viewport_height = 2.0;
    Real viewport_width = viewport_height / img.height * img.width;

    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            img(x, img.height - y - 1) = 
                normalize(Vector3{  ((x + Real(0.5)) / img.width - Real(0.5)) * viewport_width,
                                    ((y + Real(0.5)) / img.height - Real(0.5)) * viewport_height,
                                    Real(-1)});
        }
    }
    return img;
}

Image3 hw_1_2(const std::vector<std::string> &/*params*/) {
    // Homework 1.2: intersect the rays generated from hw_1_1
    // with a unit sphere located at (0, 0, -2)

    Image3 img(640 /* width */, 480 /* height */);
    Real viewport_height = 2.0;
    Real viewport_width = viewport_height / img.height * img.width;
    Sphere s = {Vector3{ 0.0, 0.0, -2.0}, 1.0, 0};

    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            Ray r = {Vector3{0,0,0}, 
                     Vector3{((x + Real(0.5)) / img.width - Real(0.5)) * viewport_width,
                            ((y + Real(0.5)) / img.height - Real(0.5)) * viewport_height,
                            Real(-1)},
                     epsilon,
                     infinity<Real>()};

            std::optional<Intersection> v_ = sphere_intersect(s, r);
            img(x, img.height - y - 1) = v_ ? (v_->normal+Real(1))/Real(2) : Vector3{0.5, 0.5, 0.5};
        }
    }
    return img;
}

Image3 hw_1_3(const std::vector<std::string> &params) {
    // Homework 1.3: add camera control to hw_1_2. 
    // We will use a look at transform:
    // The inputs are "lookfrom" (camera position),
    //                "lookat" (target),
    //                and the up vector
    // and the vertical field of view (in degrees).
    // If the user did not specify, fall back to the default
    // values below.
    // If you use the default values, it should render
    // the same image as hw_1_2.

    Vector3 lookfrom = Vector3{0, 0,  0};
    Vector3 lookat   = Vector3{0, 0, -2};
    Vector3 up       = Vector3{0, 1,  0};
    Real    vfov     = 90;
    for (int i = 0; i < (int)params.size(); i++) {
        if (params[i] == "-lookfrom") {
            Real x = std::stof(params[++i]);
            Real y = std::stof(params[++i]);
            Real z = std::stof(params[++i]);
            lookfrom = Vector3{x, y, z};
        } else if (params[i] == "-lookat") {
            Real x = std::stof(params[++i]);
            Real y = std::stof(params[++i]);
            Real z = std::stof(params[++i]);
            lookat = Vector3{x, y, z};
        } else if (params[i] == "-up") {
            Real x = std::stof(params[++i]);
            Real y = std::stof(params[++i]);
            Real z = std::stof(params[++i]);
            up = Vector3{x, y, z};
        } else if (params[i] == "-vfov") {
            vfov = std::stof(params[++i]);
        }
    }

    // avoid unused warnings
    UNUSED(lookfrom);
    UNUSED(lookat);
    UNUSED(up);
    UNUSED(vfov);

    Image3 img(640 /* width */, 480 /* height */);
    Sphere s = {Vector3{ 0.0, 0.0, -2.0}, 1.0, 0};

    Real theta = vfov / 180 * c_PI;
    Real h = tan(theta/2);
    Real viewport_height = 2.0 * h;
    Real viewport_width = viewport_height / img.height * img.width;

    Vector3 w = normalize(lookfrom - lookat);
    Vector3 u = normalize(cross(up, w));
    Vector3 v = cross(w, u);

    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            Ray r = {lookfrom, 
                     u * ((x + Real(0.5)) / img.width - Real(0.5)) * viewport_width +
                     v * ((y + Real(0.5)) / img.height - Real(0.5)) * viewport_height -
                     w,
                     epsilon,
                     infinity<Real>()};

            std::optional<Intersection> v_ = sphere_intersect(s, r);
            img(x, img.height - y - 1) = v_ ? (v_->normal+Real(1))/Real(2) : Vector3{0.5, 0.5, 0.5};
        }
    }
    return img;
}

Image3 hw_1_4(const std::vector<std::string> &params) {
    // Homework 1.4: render the scenes defined in hw1_scenes.h
    // output their diffuse color directly.
    if (params.size() == 0) {
        return Image3(0, 0);
    }

    int scene_id = std::stoi(params[0]);
    UNUSED(scene_id); // avoid unused warning
    // Your scene is hw1_scenes[scene_id]

    Image3 img(640 /* width */, 480 /* height */);

    Scene scene = hw1_scenes[scene_id];
    Camera cam = scene.camera;

    Real theta = cam.vfov / 180 * c_PI;
    Real h = tan(theta/2);
    Real viewport_height = 2.0 * h;
    Real viewport_width = viewport_height / img.height * img.width;

    Vector3 w = normalize(cam.lookfrom - cam.lookat);
    Vector3 u = normalize(cross(cam.up, w));
    Vector3 v = cross(w, u);


    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            Ray r = {cam.lookfrom, 
                     u * ((x + Real(0.5)) / img.width - Real(0.5)) * viewport_width +
                     v * ((y + Real(0.5)) / img.height - Real(0.5)) * viewport_height -
                     w,
                     epsilon,
                     infinity<Real>()};

            Vector3 color = {0.5, 0.5, 0.5};
            Real t = infinity<Real>();
            for(auto& s:scene.shapes){
                std::optional<Intersection> v_ = sphere_intersect(s, r);
                if(v_ && v_->t < t){
                    t = v_->t;
                    color = scene.materials[s.material_id].color;
                }
            }
            img(x, img.height - y - 1) = color;
        }
    }
    return img;
}

Image3 hw_1_5(const std::vector<std::string> &params) {
    // Homework 1.5: render the scenes defined in hw1_scenes.h,
    // light them using the point lights in the scene.
    if (params.size() == 0) {
        return Image3(0, 0);
    }

    int scene_id = std::stoi(params[0]);
    UNUSED(scene_id); // avoid unused warning
    // Your scene is hw1_scenes[scene_id]

    Image3 img(640 /* width */, 480 /* height */);

    Scene scene = hw1_scenes[scene_id];
    Camera cam = scene.camera;

    Real theta = cam.vfov / 180 * c_PI;
    Real h = tan(theta/2);
    Real viewport_height = 2.0 * h;
    Real viewport_width = viewport_height / img.height * img.width;

    Vector3 w = normalize(cam.lookfrom - cam.lookat);
    Vector3 u = normalize(cross(cam.up, w));
    Vector3 v = cross(w, u);


    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            Ray r = {cam.lookfrom, 
                     u * ((x + Real(0.5)) / img.width - Real(0.5)) * viewport_width +
                     v * ((y + Real(0.5)) / img.height - Real(0.5)) * viewport_height -
                     w,
                     epsilon,
                     infinity<Real>()};

            Vector3 color = {0.5, 0.5, 0.5};
            std::optional<Intersection> v_ = scene_intersect(scene, r);
            if(!v_){
                img(x, img.height - y - 1) = color;
                continue;
            }
            Intersection v = *v_;
            color = {Real(0), Real(0), Real(0)};

            for(auto& l:scene.lights){
                Real d = length(l.position - v.pos);
                Vector3 light_dir = normalize(l.position - v.pos);
                Ray shadow_ray = {v.pos, light_dir, epsilon, (1 - epsilon) * d};
                if(!scene_occluded(scene, shadow_ray)){
                    Vector3& Kd = scene.materials[v.material_id].color;
                    Vector3 n = dot(r.dir, v.normal) > 0 ? -v.normal : v.normal;
                    color += Kd * max(dot(n, light_dir), Real(0)) * l.intensity / (c_PI * d * d);
                }
            }
            img(x, img.height - y - 1) = color;
        }
    }
    return img;
}

Image3 hw_1_6(const std::vector<std::string> &params) {
    // Homework 1.6: add antialiasing to homework 1.5
    if (params.size() == 0) {
        return Image3(0, 0);
    }

    int scene_id = 0;
    int spp = 64;
    for (int i = 0; i < (int)params.size(); i++) {
        if (params[i] == "-spp") {
            spp = std::stoi(params[++i]);
        } else {
            scene_id = std::stoi(params[i]);
        }
    }

    UNUSED(scene_id); // avoid unused warning
    UNUSED(spp); // avoid unused warning
    // Your scene is hw1_scenes[scene_id]

    Image3 img(160 /* width */, 120 /* height */);

    Scene scene = hw1_scenes[scene_id];
    Camera cam = scene.camera;

    Real theta = cam.vfov / 180 * c_PI;
    Real h = tan(theta/2);
    Real viewport_height = 2.0 * h;
    Real viewport_width = viewport_height / img.height * img.width;

    Vector3 w = normalize(cam.lookfrom - cam.lookat);
    Vector3 u = normalize(cross(cam.up, w));
    Vector3 v = cross(w, u);

    std::mt19937 rng{std::random_device{}()};

    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            Vector3 color_sum = {0, 0, 0};
            for (int i = 0; i < spp; i++){
                Ray r = {cam.lookfrom, 
                        u * ((x + random_double(rng)) / img.width - Real(0.5)) * viewport_width +
                        v * ((y + random_double(rng)) / img.height - Real(0.5)) * viewport_height -
                        w,
                        epsilon,
                        infinity<Real>()};

                Vector3 color = {0.5, 0.5, 0.5};
                std::optional<Intersection> v_ = scene_intersect(scene, r);
                if(!v_){
                    color_sum += color;
                    continue;
                }
                Intersection v = *v_;
                color = {Real(0), Real(0), Real(0)};

                for(auto& l:scene.lights){
                    Real d = length(l.position - v.pos);
                    Vector3 light_dir = normalize(l.position - v.pos);
                    Ray shadow_ray = {v.pos, light_dir, epsilon, (1 - epsilon) * d};
                    if(!scene_occluded(scene, shadow_ray)){
                        Vector3& Kd = scene.materials[v.material_id].color;
                        Vector3 n = dot(r.dir, v.normal) > 0 ? -v.normal : v.normal;
                        color += Kd * max(dot(n, light_dir), Real(0)) * l.intensity / (c_PI * d * d);
                    }
                }
                color_sum += color;
            }
            img(x, img.height - y - 1) = color_sum / Real(spp);
        }
    }
    return img;
}

Image3 hw_1_7(const std::vector<std::string> &params) {
    // Homework 1.7: add mirror materials to homework 1.6
    if (params.size() == 0) {
        return Image3(0, 0);
    }

    int scene_id = 0;
    int spp = 64;
    for (int i = 0; i < (int)params.size(); i++) {
        if (params[i] == "-spp") {
            spp = std::stoi(params[++i]);
        } else {
            scene_id = std::stoi(params[i]);
        }
    }

    UNUSED(scene_id); // avoid unused warning
    UNUSED(spp); // avoid unused warning
    // Your scene is hw1_scenes[scene_id]

    Image3 img(640 /* width */, 480 /* height */);

    Scene scene = hw1_scenes[scene_id];
    Camera cam = scene.camera;

    Real theta = cam.vfov / 180 * c_PI;
    Real h = tan(theta/2);
    Real viewport_height = 2.0 * h;
    Real viewport_width = viewport_height / img.height * img.width;

    Vector3 w = normalize(cam.lookfrom - cam.lookat);
    Vector3 u = normalize(cross(cam.up, w));
    Vector3 v = cross(w, u);

    std::mt19937 rng{std::random_device{}()};

    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            Vector3 color = {0, 0, 0};
            for (int i = 0; i < spp; i++){
                Ray r = {cam.lookfrom, 
                        u * ((x + random_double(rng)) / img.width - Real(0.5)) * viewport_width +
                        v * ((y + random_double(rng)) / img.height - Real(0.5)) * viewport_height -
                        w,
                        epsilon,
                        infinity<Real>()};
                color += trace_ray(scene, r);
            }
            img(x, img.height - y - 1) = color / Real(spp);
        }
    }
    return img;
}

Image3 hw_1_8(const std::vector<std::string> &params) {
    // Homework 1.8: parallelize HW 1.7
    if (params.size() == 0) {
        return Image3(0, 0);
    }

    int scene_id = 0;
    int spp = 64;
    for (int i = 0; i < (int)params.size(); i++) {
        if (params[i] == "-spp") {
            spp = std::stoi(params[++i]);
        } else {
            scene_id = std::stoi(params[i]);
        }
    }

    UNUSED(scene_id); // avoid unused warning
    UNUSED(spp); // avoid unused warning
    // Your scene is hw1_scenes[scene_id]

    Image3 img(1280 /* width */, scene_id == 5 ? 1280 : 960 /* height */);

    Scene scene = hw1_scenes[scene_id];
    Camera cam = scene.camera;

    Real theta = cam.vfov / 180 * c_PI;
    Real h = tan(theta/2);
    Real viewport_height = 2.0 * h;
    Real viewport_width = viewport_height / img.height * img.width;

    Vector3 w = normalize(cam.lookfrom - cam.lookat);
    Vector3 u = normalize(cross(cam.up, w));
    Vector3 v = cross(w, u);

    constexpr int tile_size = 16;
    int num_tiles_x = (img.width + tile_size - 1) / tile_size;
    int num_tiles_y = (img.height + tile_size - 1) / tile_size;
    parallel_for([&](const Vector2i &tile) {
        std::mt19937 rng{std::random_device{}()};
        int x0 = tile[0] * tile_size;
        int x1 = min(x0 + tile_size, img.width);
        int y0 = tile[1] * tile_size;
        int y1 = min(y0 + tile_size, img.height);
        for (int y = y0; y < y1; y++) {
            for (int x = x0; x < x1; x++) {
                Vector3 color = {0, 0, 0};
                for (int i = 0; i < spp; i++){
                    Ray r = {cam.lookfrom, 
                            u * ((x + random_double(rng)) / img.width - Real(0.5)) * viewport_width +
                            v * ((y + random_double(rng)) / img.height - Real(0.5)) * viewport_height -
                            w,
                            epsilon,
                            infinity<Real>()};
                    color += trace_ray(scene, r);
                }
                img(x, img.height - y - 1) = color / Real(spp);
            }
        }
    }, Vector2i(num_tiles_x, num_tiles_y));
    return img;
}
