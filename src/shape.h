#pragma once
#include <variant>
#include <optional>
#include "vector.h"
#include "intersection.h"
#include "ray.h"

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

    if (a > -c_EPSILON && a < c_EPSILON)
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