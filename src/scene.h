#pragma once
#include "material.h"
#include "shape.h"
#include "light.h"
#include "bvh.h"
#include "camera.h"
#include "parse_scene.h"

struct RenderOptions {
    int spp = 4;
    int max_depth = -1;
};

struct Scene {
    Scene();
    Scene(const ParsedScene &scene);

    Camera camera;
    int width, height;
    std::vector<Shape> shapes;
    std::vector<TriangleMesh> meshes;
    std::vector<Light> lights;
    std::vector<Real> lights_power_pmf;
    std::vector<Real> lights_power_cdf;
    std::vector<Material> materials;
    TexturePool textures;
    Vector3 background_color;
    RenderOptions options;

    std::vector<BVHNode> bvh_nodes;
    int bvh_root_id;
};

std::optional<Intersection> bvh_intersect(const Scene &scene, const BVHNode &node, Ray ray);
std::optional<Intersection> scene_intersect(const Scene& scene, const Ray& r);
Real light_power(const Scene &scene, const Light &light);
bool scene_occluded(const Scene& scene, const Ray& r);
Vector3 trace_ray(const Scene& scene, const Ray& r, std::mt19937& rng);
Vector3 trace_ray_MIS(const Scene& scene, const Ray& ray, std::mt19937& rng);
Vector3 trace_ray_MIS_power(const Scene& scene, const Ray& ray, std::mt19937& rng);
void build_bvh(Scene& scene);

inline void debug_log(Scene& scene) {
    
    printf("scene.bvh_nodes[0].left_node_id: %d\n", scene.bvh_nodes[0].left_node_id);
    printf("scene.bvh_root_id: %d\n", scene.bvh_root_id);

    Light &l = scene.lights[0];
    if (auto* ll = std::get_if<PointLight>(&l))
        printf("l[0] intensity.x: %f\n", ll->intensity.x);
    else if (auto* ll = std::get_if<DiffuseAreaLight>(&l))
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
    printf("scene.num_lights: %d\n", scene.lights.size());

    Shape &s = scene.shapes[0];
    if(auto *ss = std::get_if<Sphere>(&s))
        printf("shape[0] sphere->center.x: %f\n", ss->center.x);
    else if(auto *ss = std::get_if<Triangle>(&s))
        printf("shape[0] tri->face_index: %d\n", ss->face_index);
    else
        printf("unknown shape\n");

    printf("scene.textures.image3s[0].data[0].x: %f\n", scene.textures.image3s[0].data[0].x);

    printf("scene_info.background_color.x: %f\n", scene.background_color.x);
    printf("scene_info.camera.lookat.x: %f\n", scene.camera.lookat.x);
    printf("scene_info.height: %d\n", scene.height);
    printf("spp: %d\n", scene.options.spp);
}