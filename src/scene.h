#pragma once
#include "material.h"
#include "shape.h"
#include "light.h"
#include "bvh.h"
#include "camera.h"
#include "parse_scene.h"

struct Scene {
    Scene();
    Scene(const ParsedScene &scene);

    Camera camera;
    int width, height;
    std::vector<Shape> shapes;
    std::vector<Material> materials;
    std::vector<Light> lights;
    Vector3 background_color;
    int samples_per_pixel;
    std::vector<TriangleMesh> meshes;
    TexturePool textures;

    std::vector<BVHNode> bvh_nodes;
    int bvh_root_id;
};

std::optional<Intersection> bvh_intersect(const Scene &scene, const BVHNode &node, Ray ray);
std::optional<Intersection> scene_intersect(const Scene& scene, const Ray& r);
bool scene_occluded(const Scene& scene, const Ray& r);
Vector3 trace_ray_without_sample(const Scene& scene, const Ray& r);
Vector3 trace_ray(const Scene& scene, const Ray& r, std::mt19937& rng);
void build_bvh(Scene& scene);