#pragma once
#include "material.h"
#include "shape.h"
#include "light.h"
#include "bvh.h"
#include "camera.h"

struct RenderOptions {
    int spp = 4;
    int max_depth = -1;
};

struct BSphere {
    Real radius;
    Vector3 center;
};

struct Scene {
    // Scene();
    // Scene(const Scene& scene);

    Camera camera;
    std::vector<Shape> shapes;
    std::vector<TriangleMesh> meshes;
    std::vector<Light> lights;
    std::vector<Material> materials;
    TexturePool textures;
    Vector3 background_color;
    int envmap_light_id;

    RenderOptions options;
    std::string output_filename;

    std::vector<Real> lights_power_pmf;
    std::vector<Real> lights_power_cdf;

    BSphere bounds;
    std::vector<BVHNode> bvh_nodes;
    int bvh_root_id;
};

std::optional<Intersection> scene_intersect(const Scene& scene, const Ray& r);
bool scene_occluded(const Scene& scene, const Ray& r);
void build_bvh(Scene& scene);

inline bool has_envmap(const Scene &scene) {
    return scene.envmap_light_id != -1;
}

inline const Light& get_envmap(const Scene &scene) {
    assert(scene.envmap_light_id != -1);
    return scene.lights[scene.envmap_light_id];
}

inline void debug_log(Scene& scene) {
    
    printf("scene.bvh_nodes[0].left_node_id: %d\n", scene.bvh_nodes[0].left_node_id);
    printf("scene.bvh_root_id: %d\n", scene.bvh_root_id);

    Light &l = scene.lights[0];
    if (auto* ll = std::get_if<PointLight>(&l))
        printf("l[0] intensity.x: %f\n", ll->intensity.x);
    else if (auto* ll = std::get_if<AreaLight>(&l))
        printf("l[0] intensity.x: %f\n", ll->intensity.x);

    Material &m = scene.materials[0];
    if(auto *mm = std::get_if<Diffuse>(&m))
        printf("m[0] color.x: %f\n", eval(mm->reflectance, Vector2{0, 0}, scene.textures).x);
    else if(auto *mm = std::get_if<Mirror>(&m))
        printf("m[0] color.x: %f\n", eval(mm->reflectance, Vector2{0, 0}, scene.textures).x);
    else if(auto *mm = std::get_if<Plastic>(&m))
        printf("m[0] color.x: %f\n", eval(mm->reflectance, Vector2{0, 0}, scene.textures).x);
    else if(auto *mm = std::get_if<Phong>(&m))
        printf("m[0] color.x: %f\n", eval(mm->reflectance, Vector2{0, 0}, scene.textures).x);
    else if(auto *mm = std::get_if<BlinnPhong>(&m))
        printf("m[0] color.x: %f\n", eval(mm->reflectance, Vector2{0, 0}, scene.textures).x);
    else if(auto *mm = std::get_if<BlinnPhongMicrofacet>(&m))
        printf("m[0] color.x: %f\n", eval(mm->reflectance, Vector2{0, 0}, scene.textures).x);
    else
        printf("unknown material\n");

    printf("scene.meshes[0].normals[0].xyz: %f %f %f\n", scene.meshes[0].normals[0].x, scene.meshes[0].normals[0].y, scene.meshes[0].normals[0].z);
    printf("scene.num_lights: %d\n", static_cast<int>(scene.lights.size()));

    Shape &s = scene.shapes[0];
    if(auto *ss = std::get_if<Sphere>(&s))
        printf("shape[0] sphere->center.x: %f\n", ss->center.x);
    else if(auto *ss = std::get_if<Triangle>(&s))
        printf("shape[0] tri->face_index: %d\n", ss->face_id);
    else
        printf("unknown shape\n");

    printf("scene.textures.image3s[0].data[0].x: %f\n", scene.textures.image3s[0].data[0].x);

    printf("scene_info.background_color.x: %f\n", scene.background_color.x);
    printf("scene_info.camera.lookat.x: %f\n", scene.camera.lookat.x);
    printf("scene_info.height: %d\n", scene.camera.height);
    printf("spp: %d\n", scene.options.spp);
}