#pragma once

#include <optional>
#include <variant>
#include <random>

#include "vector.h"
#include "parse_scene.h"

namespace hw2 {
    const Real epsilon = 1e-4;

    inline double random_double(std::mt19937 &rng) {
        return std::uniform_real_distribution<double>{0.0, 1.0}(rng);
    }

    inline int random_int(int min, int max, std::mt19937 &rng) {
        return static_cast<int>(min + (max - min) * random_double(rng));
    }

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

    // Ray
    struct Ray {
        Vector3 origin;
        Vector3 dir;
        Real tmin;
        Real tmax;
    };

    // AABB
    struct AABB {
        AABB() {}
        AABB(const Vector3& a, const Vector3& b) { pmin = a; pmax = b;}

        bool hit(const Ray& r) const {
            Real t_min = r.tmin;
            Real t_max = r.tmax;
            for (int a = 0; a < 3; a++) {
                auto t0 = fmin((pmin[a] - r.origin[a]) / r.dir[a],
                               (pmax[a] - r.origin[a]) / r.dir[a]);
                auto t1 = fmax((pmin[a] - r.origin[a]) / r.dir[a],
                               (pmax[a] - r.origin[a]) / r.dir[a]);
                t_min = fmax(t0, t_min);
                t_max = fmin(t1, t_max);
                if (t_max < t_min)
                    return false;
            }
            return true;
        }

        Vector3 pmin;
        Vector3 pmax;
    };

    inline AABB Union(const AABB& b1, const AABB& b2){
        AABB ret;
        ret.pmin = min(b1.pmin, b2.pmin);
        ret.pmax = max(b1.pmax, b2.pmax);
        return ret;
    }

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

    struct get_aabb_op {
        AABB operator()(const Sphere &s) const;
        AABB operator()(const Triangle &s) const;
    };

    AABB get_aabb_op::operator()(const Sphere& s) const {
        return {s.center - Vector3(s.radius, s.radius, s.radius), s.center + Vector3(s.radius, s.radius, s.radius)};
    }

    AABB get_aabb_op::operator()(const Triangle &s) const {
        AABB ret;
        const TriangleMesh &mesh = *s.mesh;
        const Vector3 &indices = mesh.indices.at(s.face_index);

        Vector3 v0 = mesh.positions.at(indices.x);
        Vector3 v1 = mesh.positions.at(indices.y);
        Vector3 v2 = mesh.positions.at(indices.z);
        
        ret.pmin = min(min(v0, v1), v2);
        ret.pmax = max(max(v0, v1), v2);
        return ret;
    }

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

        if (t < r.tmin || r.tmax < t) // ray intersection
            return {};
        else {
            Intersection inter;
            inter.t = t;
            inter.pos = r.origin + r.dir * t;
            inter.normal = normalize(cross(e1, e2));
            inter.material_id = mesh.material_id;
            inter.uv = Vector2(u, v);
            return inter;
        }
    }

    // BVH
    struct BVHNode {
        BVHNode();
        BVHNode(const std::vector<Shape*>& src_shapes, std::mt19937 &rng)
            : BVHNode(src_shapes, 0, src_shapes.size(), rng)
        {}
        BVHNode(const std::vector<Shape*>& src_shapes, size_t start, size_t end, std::mt19937 &rng);

        std::optional<Intersection> intersect(const Ray& r) const;

        std::unique_ptr<BVHNode> left;
        std::unique_ptr<BVHNode> right;
        Shape *shape = nullptr;
        AABB box;
    };

    inline bool box_compare(const Shape* a, const Shape* b, int axis) {
        AABB abox = std::visit(get_aabb_op{}, *a);
        AABB bbox = std::visit(get_aabb_op{}, *b);
        return abox.pmin[axis] < bbox.pmin[axis];
    }


    bool box_x_compare (const Shape* a, const Shape* b) {
        return box_compare(a, b, 0);
    }

    bool box_y_compare (const Shape* a, const Shape* b) {
        return box_compare(a, b, 1);
    }

    bool box_z_compare (const Shape* a, const Shape* b) {
        return box_compare(a, b, 2);
    }

    BVHNode::BVHNode(
        const std::vector<Shape*>& src_shapes,
        size_t start, size_t end,
        std::mt19937 &rng
    ) {
        auto shapes = src_shapes;

        int axis = random_int(0,2,rng);
        auto comparator = (axis == 0) ? box_x_compare
                        : (axis == 1) ? box_y_compare
                                    : box_z_compare;

        size_t object_span = end - start;

        if (object_span == 1) {
            shape = shapes[start];
            box = std::visit(get_aabb_op{}, *shape);
            return;
        } else if (object_span == 2) {
            if (comparator(shapes[start], shapes[start+1])) {
                left = std::make_unique<BVHNode>(shapes, start, start+1, rng);
                right = std::make_unique<BVHNode>(shapes, start+1, start+2, rng);
            } else {
                left = std::make_unique<BVHNode>(shapes, start+1, start+2, rng);
                right = std::make_unique<BVHNode>(shapes, start, start+1, rng);
            }
            box = Union(left->box, right->box);
            return;
        } else {
            std::sort(shapes.begin() + start, shapes.begin() + end, comparator);
            auto mid = start + object_span/2;
            left = std::make_unique<BVHNode>(shapes, start, mid, rng);
            right = std::make_unique<BVHNode>(shapes, mid, end, rng);
        }

        box = Union(left->box, right->box);
    }

    std::optional<Intersection> BVHNode::intersect(const Ray& r) const{
        if (!box.hit(r))
            return {};

        if(!(shape == nullptr)){
            return std::visit(intersect_op{r}, *shape);
        }

        auto hit_left = left == nullptr ? std::nullopt : left->intersect(r);
        auto hit_right = right == nullptr ? std::nullopt : right->intersect(r);

        if(hit_left && hit_right)
            return hit_left->t < hit_right->t ? hit_left : hit_right;
        else
            return hit_left ? hit_left : hit_right;
    }

    // Scene
    struct Scene {
        Scene();
        Scene(const ParsedScene &scene);

        Camera camera;
        int width, height;
        std::vector<Shape> shapes;
        std::vector<Material> materials;
        std::vector<PointLight> lights;
        Vector3 background_color;
        int samples_per_pixel;
        std::vector<TriangleMesh> meshes;
        std::unique_ptr<BVHNode> bvh = nullptr;
    };

    Camera from_parsed_camera(const ParsedCamera &pc) {
        Camera c;
        c.lookat = pc.lookat;
        c.lookfrom = pc.lookfrom;
        c.up = pc.up;
        c.vfov = pc.vfov;
        return c;
    }

    Scene::Scene(){}

    Scene::Scene(const ParsedScene &scene) : camera(from_parsed_camera(scene.camera)),
                                             width(scene.camera.width),
                                             height(scene.camera.height),
                                             background_color(scene.background_color),
                                             samples_per_pixel(scene.samples_per_pixel)
    {
        // Extract triangle meshes from the parsed scene.
        int tri_mesh_count = 0;
        for (const ParsedShape &parsed_shape : scene.shapes)
        {
            if (std::get_if<ParsedTriangleMesh>(&parsed_shape))
            {
                tri_mesh_count++;
            }
        }
        meshes.resize(tri_mesh_count);
        // Extract the shapes
        tri_mesh_count = 0;
        for (int i = 0; i < (int)scene.shapes.size(); i++)
        {
            const ParsedShape &parsed_shape = scene.shapes[i];
            if (auto *sph = std::get_if<ParsedSphere>(&parsed_shape))
            {
                shapes.push_back(
                    Sphere{{sph->material_id, sph->area_light_id},
                           sph->position,
                           sph->radius});
            }
            else if (auto *parsed_mesh = std::get_if<ParsedTriangleMesh>(&parsed_shape))
            {
                meshes[tri_mesh_count] = TriangleMesh{
                    {parsed_mesh->material_id, parsed_mesh->area_light_id},
                    parsed_mesh->positions,
                    parsed_mesh->indices,
                    parsed_mesh->normals,
                    parsed_mesh->uvs};
                // Extract all the individual triangles
                for (int face_index = 0; face_index < (int)parsed_mesh->indices.size(); face_index++)
                {
                    shapes.push_back(Triangle{face_index, &meshes[tri_mesh_count]});
                }
                tri_mesh_count++;
            }
            else
            {
                assert(false);
            }
        }
        // Copy the materials
        for (const ParsedMaterial &parsed_mat : scene.materials)
        {
            if (auto *diffuse = std::get_if<ParsedDiffuse>(&parsed_mat))
            {
                // We assume the reflectance is always RGB for now.
                materials.push_back(Material{MaterialType::Diffuse, std::get<Vector3>(diffuse->reflectance)});
            }
            else if (auto *mirror = std::get_if<ParsedMirror>(&parsed_mat))
            {
                // We assume the reflectance is always RGB for now.
                materials.push_back(Material{MaterialType::Mirror, std::get<Vector3>(mirror->reflectance)});
            }
            else
            {
                assert(false);
            }
        }
        for (const ParsedLight &parsed_light : scene.lights)
        {
            // We assume all lights are point lights for now.
            ParsedPointLight point_light = std::get<ParsedPointLight>(parsed_light);
            lights.push_back(PointLight{point_light.intensity, point_light.position});
        }
    }

    std::optional<Intersection> scene_intersect(const Scene& scene, const Ray& r){
        if(scene.bvh){
            return scene.bvh->intersect(r);
        }else{
            // Traverse
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
    }

    bool scene_occluded(const Scene& scene, const Ray& r){
        if(scene.bvh){
            std::optional<Intersection> v_ = scene.bvh->intersect(r);
            return v_ ? true : false;
        }else{
            Real t = infinity<Real>();
            for(auto& s:scene.shapes){
                std::optional<Intersection> v_ = std::visit(intersect_op{r}, s);
                if(v_ && v_->t < t)
                    t = v_->t;
            }
            return t < infinity<Real>();
        }
    }

    Vector3 trace_ray(const Scene& scene, const Ray& r){
        std::optional<Intersection> v_ = scene_intersect(scene, r);
        if(!v_) return scene.background_color;
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
            return scene.background_color;
        }
    }
}