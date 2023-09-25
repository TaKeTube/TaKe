#pragma once

#include "take.h"
#include "vector.h"
#include "scene.h"

#include <filesystem>
#include <variant>
#include <vector>

inline void set_material_id(TriangleMesh &mesh, int material_id) {
    mesh.material_id = material_id;
}
inline void set_material_id(Shape &shape, int material_id) {
    std::visit([&](auto &s) { s.material_id = material_id; }, shape);
}
inline void set_area_light_id(Shape &shape, int area_light_id) {
    std::visit([&](auto &s) { s.area_light_id = area_light_id; }, shape);
}
inline int get_material_id(const Shape &shape) {
    return std::visit([&](const auto &s) { return s.material_id; }, shape);
}
inline int get_area_light_id(const Shape &shape) {
    return std::visit([&](const auto &s) { return s.area_light_id; }, shape);
}
inline bool is_light(const Shape &shape) {
    return get_area_light_id(shape) >= 0;
}

Scene parse_scene(const fs::path &filename);
