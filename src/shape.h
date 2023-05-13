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