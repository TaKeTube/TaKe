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

struct Triangle : public ShapeBase {
    int face_id;
    int mesh_id;
};

Vector2 get_sphere_uv(const Vector3& p);

using Shape = std::variant<Sphere, Triangle>;

struct intersect_op {
    std::optional<Intersection> operator()(const Sphere &s) const;
    std::optional<Intersection> operator()(const Triangle &s) const;

    const std::vector<TriangleMesh>& meshes;
    const Ray& r;
};

inline std::optional<Intersection> intersect_shape(const Shape& shape, const std::vector<TriangleMesh>& meshes, const Ray& r){
    return std::visit(intersect_op{meshes, r}, shape);
}

struct sample_on_shape_op {
    PointAndNormal operator()(const Sphere &s) const;
    PointAndNormal operator()(const Triangle &s) const;

    const std::vector<TriangleMesh>& meshes;
    const Vector3 &ref_pos;
    std::mt19937& rng;
};

inline PointAndNormal sample_on_shape(const Shape& shape, const std::vector<TriangleMesh>& meshes, const Vector3 &ref_pos, std::mt19937& rng) {
    return std::visit(sample_on_shape_op{meshes, ref_pos, rng}, shape);
}

struct sample_on_shape_pdf_op {
    Real operator()(const Sphere &s) const;
    Real operator()(const Triangle &s) const;

    const std::vector<TriangleMesh>& meshes;
    const PointAndNormal &shape_point;
    const Vector3 &ref_pos;
};

inline Real get_shape_pdf(const Shape &s, 
                          const Vector3 &ref_pos, 
                          const PointAndNormal &shape_point, 
                          const std::vector<TriangleMesh>& meshes) {
    return std::visit(sample_on_shape_pdf_op{meshes, shape_point, ref_pos}, s);
}

struct get_area_op {
    Real operator()(const Sphere &s) const;
    Real operator()(const Triangle &s) const;

    const std::vector<TriangleMesh>& meshes;
};

inline Real get_area(const Shape& shape, const std::vector<TriangleMesh>& meshes) {
    return std::visit(get_area_op{meshes}, shape);
}