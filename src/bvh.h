#pragma once
#include "bbox.h"
#include "shape.h"

struct BVHNode {
    BBox box;
    int left_node_id;
    int right_node_id;
    int primitive_id;
};

int construct_bvh(const std::vector<BBoxWithID> &boxes, std::vector<BVHNode> &node_pool);
std::optional<Intersection> bvh_intersect(const int bvh_root_id, const std::vector<BVHNode> &bvh_nodes, const std::vector<Shape> &shapes, const std::vector<TriangleMesh>& meshes, Ray ray);