#include "scene.h"

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

    std::vector<BBoxWithID> bboxes(shapes.size());
    for (int i = 0; i < (int)bboxes.size(); i++) {
        if (auto *sph = std::get_if<Sphere>(&shapes[i])) {
            Vector3 p_min = sph->center - sph->radius;
            Vector3 p_max = sph->center + sph->radius;
            bboxes[i] = {BBox{p_min, p_max}, i};
        } else if (auto *tri = std::get_if<Triangle>(&shapes[i])) {
            const TriangleMesh *mesh = tri->mesh;
            Vector3i index = mesh->indices[tri->face_index];
            Vector3 p0 = mesh->positions[index[0]];
            Vector3 p1 = mesh->positions[index[1]];
            Vector3 p2 = mesh->positions[index[2]];
            Vector3 p_min = min(min(p0, p1), p2);
            Vector3 p_max = max(max(p0, p1), p2);
            bboxes[i] = {BBox{p_min, p_max}, i};
        }
    }
    bvh_root_id = construct_bvh(bboxes, bvh_nodes);
}

std::optional<Intersection> scene_intersect(const Scene& scene, const Ray& r){
    if(!scene.bvh_nodes.empty()){
        return bvh_intersect(scene, scene.bvh_nodes[scene.bvh_root_id], r);
        // Intersection v;
        // scene.bvh->intersect(r, v);
        // return v;
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
    if(!scene.bvh_nodes.empty()){
        std::optional<Intersection> v_ = bvh_intersect(scene, scene.bvh_nodes[scene.bvh_root_id], r);
        return v_ ? true : false;
        // Intersection v;
        // return scene.bvh->intersect(r, v);
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
    // Vector3 n = v.normal;

    if(scene.materials[v.material_id].type == MaterialType::Diffuse){
        Vector3 color = {Real(0), Real(0), Real(0)};
        for(auto& l:scene.lights){
            Real d = length(l.position - v.pos);
            Vector3 light_dir = normalize(l.position - v.pos);
            Ray shadow_ray = {v.pos, light_dir, c_EPSILON, (1 - c_EPSILON) * d};
            if(!scene_occluded(scene, shadow_ray)){
                const Vector3& Kd = scene.materials[v.material_id].color;
                color += Kd * max(dot(n, light_dir), Real(0)) * l.intensity / (c_PI * d * d);
            }
        }
        return color;
    }else if(scene.materials[v.material_id].type == MaterialType::Mirror){
        Ray reflect_ray = {v.pos, r.dir - 2*dot(r.dir, n) * n, c_EPSILON, infinity<Real>()};
        return scene.materials[v.material_id].color * trace_ray(scene, reflect_ray);
    }else{
        return scene.background_color;
    }
}