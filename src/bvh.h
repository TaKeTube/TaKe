#pragma once
#include "bbox.h"
#include "scene.h"

struct BVHNode {
    BBox box;
    int left_node_id;
    int right_node_id;
    int primitive_id;
};

int construct_bvh(const std::vector<BBoxWithID> &boxes,
                  std::vector<BVHNode> &node_pool);

std::optional<Intersection> bvh_intersect(const Scene &scene, const BVHNode &node, Ray ray);