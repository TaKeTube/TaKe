#include "bvh.h"
#include <optional>
#include <variant>
#include "intersection.h"

#define BVH_STACK_DEPTH 64

int construct_bvh(const std::vector<BBoxWithID> &boxes,
                std::vector<BVHNode> &node_pool) {
    if (boxes.size() == 1) {
        BVHNode node;
        node.left_node_id = node.right_node_id = -1;
        node.primitive_id = boxes[0].id;
        node.box = boxes[0].box;
        node_pool.push_back(node);
        return node_pool.size() - 1;
    }

    BBox big_box;
    for (const BBoxWithID &b : boxes) {
        big_box = merge(big_box, b.box);
    }
    int axis = largest_axis(big_box);
    std::vector<BBoxWithID> local_boxes = boxes;
    std::sort(local_boxes.begin(), local_boxes.end(),
        [&](const BBoxWithID &b1, const BBoxWithID &b2) {
            Vector3 center1 = (b1.box.p_max + b1.box.p_min) / Real(2);
            Vector3 center2 = (b2.box.p_max + b2.box.p_min) / Real(2);
            return center1[axis] < center2[axis];
        });
    std::vector<BBoxWithID> left_boxes(
        local_boxes.begin(),
        local_boxes.begin() + local_boxes.size() / 2);
    std::vector<BBoxWithID> right_boxes(
        local_boxes.begin() + local_boxes.size() / 2,
        local_boxes.end());

    BVHNode node;
    node.box = big_box;
    node.left_node_id = construct_bvh(left_boxes, node_pool);
    node.right_node_id = construct_bvh(right_boxes, node_pool);
    node.primitive_id = -1;
    node_pool.push_back(node);
    return node_pool.size() - 1;
}

// Tested on party_bgonly.xml, traverse version is faster than recursive version by around 2-3 s
// Traverse version
std::optional<Intersection> bvh_intersect(const int bvh_root_id, const std::vector<BVHNode> &bvh_nodes, const std::vector<Shape> &shapes, Ray ray) {
    int node_ptr = 0;
    bool is_left = true;
    std::optional<Intersection> intersection;
    intersection->t = infinity<Real>();
    unsigned int bvhStack[BVH_STACK_DEPTH];
    bvhStack[++node_ptr] = bvh_root_id;

    while(node_ptr)
    {
        BVHNode curr_node = bvh_nodes[bvhStack[node_ptr--]];

        if(!intersect(curr_node.box, ray))
            continue;
        
        if(curr_node.primitive_id != -1)
        {
            std::optional<Intersection> temp_intersection = std::visit(intersect_op{ray}, shapes[curr_node.primitive_id]);
            if(temp_intersection && temp_intersection->t < intersection->t){
                intersection = std::move(temp_intersection);
                if(is_left && intersection->t < ray.tmax) ray.tmax = intersection->t;
            }
            if(temp_intersection)
                is_left = false;
        }
        else
        {
            bvhStack[++node_ptr] = curr_node.right_node_id;
            bvhStack[++node_ptr] = curr_node.left_node_id;
            is_left = true;
        }
    }
    return std::isfinite<Real>(intersection->t) ? intersection : std::nullopt;
}

// Recursive version
// std::optional<Intersection> bvh_intersect(const std::vector<BVHNode> &bvh_nodes, const std::vector<Shape> &shapes, const BVHNode &node, Ray ray) {
//     if (node.primitive_id != -1) {
//         return std::visit(intersect_op{ray}, shapes[node.primitive_id]);
//     }
//     const BVHNode &left = bvh_nodes[node.left_node_id];
//     const BVHNode &right = bvh_nodes[node.right_node_id];
//     std::optional<Intersection> isect_left;
//     if (intersect(left.box, ray)) {
//         isect_left = bvh_intersect(bvh_nodes, shapes, left, ray);
//         if (isect_left) {
//             ray.tmax = isect_left->t;
//         }
//     }
//     if (intersect(right.box, ray)) {
//         // Since we've already set ray.tfar to the left node
//         // if we still hit something on the right, it's closer
//         // and we should return that.
//         if (auto isect_right = bvh_intersect(bvh_nodes, shapes, right, ray)) {
//             return isect_right;
//         }
//     }
//     return isect_left;
// }