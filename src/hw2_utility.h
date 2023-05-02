#pragma once

#include <optional>
#include <variant>
#include <random>

#include "vector.h"

namespace hw2 {
    const Real epsilon = 1e-4;

    enum class MaterialType {
        Diffuse,
        Mirror
    };

    struct Camera {
        Vector3 lookfrom;
        Vector3 lookat;
        Vector3 up;
        Real vfov;
    };

    struct PointLight {
        Vector3 intensity;
        Vector3 position;    
    };

    struct Material {
        MaterialType type;
        Vector3 color;
    };

    // Shapes
    struct ShapeBase {
        int material_id = -1;
        int area_light_id = -1;
    };

    struct TriangleMesh : public ShapeBase {
        std::vector<Vector3> positions;
        std::vector<Vector3i> indices;
        std::vector<Vector3> normals;
        std::vector<Vector2> uvs;
    };

    struct Sphere : public ShapeBase {
        Vector3 center;
        Real radius;
    };

    struct Triangle {
        int face_index;
        const TriangleMesh *mesh;
    };


    using Shape = std::variant<Sphere, Triangle>;

    // Scene
    struct Scene {
        Camera camera;
        int width, height;
        std::vector<Shape> shapes;
        std::vector<Material> materials;
        std::vector<PointLight> lights;
        Vector3 background_color;
        int samples_per_pixel;
        std::vector<TriangleMesh> meshes;
    };

    // Ray
    struct Ray {
        Vector3 origin;
        Vector3 dir;
        Real tmin;
        Real tmax;
    };

    // Intersection
    struct Intersection {
        Vector3 pos;
        Vector3 normal;
        Vector2 uv;
        Real t;
        int material_id;
    };

    struct intersect_op {
        std::optional<Intersection> operator()(const Sphere &s) const;
        std::optional<Intersection> operator()(const Triangle &s) const;

        const Ray& r;
    };

    std::optional<Intersection> intersect_op::operator()(const Sphere& s) const {
        Vector3 oc = r.origin - s.center;
        Real a = dot(r.dir, r.dir);
        Real half_b = dot(oc, r.dir);
        Real c = dot(oc, oc) - s.radius*s.radius;

        Real discriminant = half_b*half_b - a*c;
        if (discriminant < 0) 
            return {};
        Real sqrtd = sqrt(discriminant);

        Real root = (-half_b - sqrtd) / a;
        if (root < r.tmin || r.tmax < root) {
            root = (-half_b + sqrtd) / a;
            if (root < r.tmin || r.tmax < root)
                return {};
        }

        Intersection v;
        v.t = root;
        v.pos = r.origin + r.dir * v.t;
        v.normal = (v.pos - s.center) / s.radius;
        v.material_id = s.material_id;

        return v;
    }

    std::optional<Intersection> intersect_op::operator()(const Triangle& tri) const {
        const TriangleMesh &mesh = *tri.mesh;
        const Vector3 &indices = mesh.indices.at(tri.face_index);

        Vector3 v0 = mesh.positions.at(indices.x);
        Vector3 v1 = mesh.positions.at(indices.y);
        Vector3 v2 = mesh.positions.at(indices.z);
        Vector3 e1, e2, h, s, q;
        Real a, f, u, v;
        e1 = v1 - v0;
        e2 = v2 - v0;
        h = cross(r.dir, e2);
        a = dot(e1, h);

        if (a > -epsilon && a < epsilon)
            return {};    // This ray is parallel to this triangle.

        f = 1.0 / a;
        s = r.origin - v0;
        u = f * dot(s, h);

        if (u < 0.0 || u > 1.0)
            return {};

        q = cross(s, e1);
        v = f * dot(r.dir, q);

        if (v < 0.0 || u + v > 1.0)
            return {};

        // At this stage we can compute t to find out where the intersection point is on the line.
        Real t = f * dot(e2, q);

        if (t > epsilon) // ray intersection
        {
            Intersection inter;
            inter.t = t;
            inter.pos = r.origin + r.dir * t;
            inter.normal = normalize(cross(e1, e2));
            inter.material_id = mesh.material_id;
            inter.uv = Vector2(u, v);
            return inter;
        }
        else // This means that there is a line intersection but not a ray intersection.
            return {};
    }

    std::optional<Intersection> scene_intersect(const Scene& scene, const Ray& r){
        Real t = infinity<Real>();
        Intersection v = {};
        for(auto& s:scene.shapes){
            std::optional<Intersection> v_ = std::visit(intersect_op{r}, s);
            if(v_ && v_->t < t){
                t = v_->t;
                v = *v_;
            }
        }
        if(t < infinity<Real>())
            return v;
        else
            return {};
    }

    bool scene_occluded(const Scene& scene, const Ray& r){
        Real t = infinity<Real>();
        for(auto& s:scene.shapes){
            std::optional<Intersection> v_ = std::visit(intersect_op{r}, s);
            if(v_ && v_->t < t)
                t = v_->t;
        }
        return t < infinity<Real>();
    }

    inline double random_double(std::mt19937 &rng) {
        return std::uniform_real_distribution<double>{0.0, 1.0}(rng);
    }

    Vector3 trace_ray(const Scene& scene, const Ray& r){
        std::optional<Intersection> v_ = scene_intersect(scene, r);
        if(!v_) return {0.5, 0.5, 0.5};
        Intersection v = *v_;
        Vector3 n = dot(r.dir, v.normal) > 0 ? -v.normal : v.normal;

        if(scene.materials[v.material_id].type == MaterialType::Diffuse){
            Vector3 color = {Real(0), Real(0), Real(0)};
            for(auto& l:scene.lights){
                Real d = length(l.position - v.pos);
                Vector3 light_dir = normalize(l.position - v.pos);
                Ray shadow_ray = {v.pos, light_dir, epsilon, (1 - epsilon) * d};
                if(!scene_occluded(scene, shadow_ray)){
                    const Vector3& Kd = scene.materials[v.material_id].color;
                    color += Kd * max(dot(n, light_dir), Real(0)) * l.intensity / (c_PI * d * d);
                }
            }
            return color;
        }else if(scene.materials[v.material_id].type == MaterialType::Mirror){
            Ray reflect_ray = {v.pos, r.dir - 2*dot(r.dir, n) * n, epsilon, infinity<Real>()};
            return scene.materials[v.material_id].color * trace_ray(scene, reflect_ray);
        }else{
            return {0.5, 0.5, 0.5};
        }
    }
}