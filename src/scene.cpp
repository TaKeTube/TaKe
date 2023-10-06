#include "scene.h"
#include "parse/parse_scene.h"

void build_bvh(Scene& scene) {
    std::vector<BBoxWithID> bboxes(scene.shapes.size());
    for (int i = 0; i < (int)bboxes.size(); i++) {
        if (auto *sph = std::get_if<Sphere>(&scene.shapes[i])) {
            Vector3 p_min = sph->center - sph->radius;
            Vector3 p_max = sph->center + sph->radius;
            bboxes[i] = {BBox{p_min, p_max}, i};
        } else if (auto *tri = std::get_if<Triangle>(&scene.shapes[i])) {
            const TriangleMesh &mesh = scene.meshes[tri->mesh_id];
            Vector3i index = mesh.indices[tri->face_id];
            Vector3 p0 = mesh.positions[index[0]];
            Vector3 p1 = mesh.positions[index[1]];
            Vector3 p2 = mesh.positions[index[2]];
            Vector3 p_min = min(min(p0, p1), p2);
            Vector3 p_max = max(max(p0, p1), p2);
            bboxes[i] = {BBox{p_min, p_max}, i};
        }
    }
    scene.bvh_root_id = construct_bvh(bboxes, scene.bvh_nodes);
    Vector3 scene_pmax = scene.bvh_nodes[scene.bvh_root_id].box.p_max;
    Vector3 scene_pmin = scene.bvh_nodes[scene.bvh_root_id].box.p_min;
    scene.bounds = {distance(scene_pmax, scene_pmin) * 0.5, (scene_pmax + scene_pmin) * 0.5};
}

std::optional<Intersection> scene_intersect(const Scene& scene, const Ray& r){
    if(!scene.bvh_nodes.empty()){
        return bvh_intersect(scene.bvh_root_id, scene.bvh_nodes, scene.shapes, scene.meshes, r);
        // Intersection v;
        // scene.bvh->intersect(r, v);
        // return v;
    }else{
        // Traverse
        Real t = infinity<Real>();
        Intersection v = {};
        for(auto& s:scene.shapes){
            std::optional<Intersection> v_ = std::visit(intersect_op{scene.meshes, r}, s);
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
    if(!scene.bvh_nodes.empty()){
        std::optional<Intersection> v_ = bvh_intersect(scene.bvh_root_id, scene.bvh_nodes, scene.shapes, scene.meshes, r);
        return v_ ? true : false;
        // Intersection v;
        // return scene.bvh->intersect(r, v);
    }else{
        Real t = infinity<Real>();
        for(auto& s:scene.shapes){
            std::optional<Intersection> v_ = std::visit(intersect_op{scene.meshes, r}, s);
            if(v_ && v_->t < t)
                t = v_->t;
        }
        return t < infinity<Real>();
    }
}