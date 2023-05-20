#include "scene.h"

Scene::Scene(){}

Scene::Scene(const ParsedScene &scene) : camera(from_parsed_camera(scene.camera)),
                                            width(scene.camera.width),
                                            height(scene.camera.height),
                                            background_color(scene.background_color)
{
    options.spp = scene.samples_per_pixel;

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
                shapes.push_back(Triangle{face_index, &meshes[tri_mesh_count], parsed_mesh->area_light_id});
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
            if (auto *color = std::get_if<Vector3>(&diffuse->reflectance)) {
                ConstTexture texture = {*color};
                materials.push_back(Material{MaterialType::Diffuse, texture});
            } else if (auto *color = std::get_if<ParsedImageTexture>(&diffuse->reflectance)) {
                std::string texture_name = color->filename.string();
                int texture_id;
                auto it = textures.image3s_map.find(texture_name);
                if (it != textures.image3s_map.end()) {
                    texture_id = textures.image3s_map.at(texture_name);
                } else {
                    texture_id = textures.image3s.size();
                    textures.image3s_map.emplace(texture_name, texture_id);
                    textures.image3s.push_back(imread3(color->filename));
                }
                ImageTexture texture = {
                    texture_id,
                    color->uscale, color->vscale,
                    color->uoffset, color->voffset,
                };
                materials.push_back(Material{MaterialType::Diffuse, texture});
            }
        }
        else if (auto *mirror = std::get_if<ParsedMirror>(&parsed_mat))
        {
            if (auto *color = std::get_if<Vector3>(&mirror->reflectance)) {
                ConstTexture texture = {*color};
                materials.push_back(Material{MaterialType::Mirror, texture});
            } else if (auto *color = std::get_if<ParsedImageTexture>(&mirror->reflectance)) {
                std::string texture_name = color->filename.string();
                int texture_id;
                auto it = textures.image3s_map.find(texture_name);
                if (it != textures.image3s_map.end()) {
                    texture_id = textures.image3s_map.at(texture_name);
                } else {
                    texture_id = textures.image3s.size();
                    textures.image3s_map.emplace(texture_name, texture_id);
                    textures.image3s.push_back(imread3(color->filename));
                }
                ImageTexture texture = {
                    texture_id,
                    color->uscale, color->vscale,
                    color->uoffset, color->voffset,
                };
                materials.push_back(Material{MaterialType::Mirror, texture});
            }
        }
        else if (auto *plastic = std::get_if<ParsedPlastic>(&parsed_mat))
        {
            if (auto *color = std::get_if<Vector3>(&plastic->reflectance)) {
                ConstTexture texture = {*color};
                materials.push_back(Material{MaterialType::Plastic, texture, plastic->eta});
            } else if (auto *color = std::get_if<ParsedImageTexture>(&plastic->reflectance)) {
                std::string texture_name = color->filename.string();
                int texture_id;
                auto it = textures.image3s_map.find(texture_name);
                if (it != textures.image3s_map.end()) {
                    texture_id = textures.image3s_map.at(texture_name);
                } else {
                    texture_id = textures.image3s.size();
                    textures.image3s_map.emplace(texture_name, texture_id);
                    textures.image3s.push_back(imread3(color->filename));
                }
                ImageTexture texture = {
                    texture_id,
                    color->uscale, color->vscale,
                    color->uoffset, color->voffset,
                };
                materials.push_back(Material{MaterialType::Plastic, texture, plastic->eta});
            }
        }
        else
        {
            assert(false);
        }
    }
    for (const ParsedLight &parsed_light : scene.lights)
    {
        if (auto* point_light = std::get_if<ParsedPointLight>(&parsed_light)){
            lights.push_back(PointLight{point_light->intensity, point_light->position});
        }
    }
    // Reset shape id of area light
    for (int i = 0; i < shapes.size(); ++i) {
        int area_light_id = -1;
        Shape& shape = shapes.at(i);
        if (auto *sph = std::get_if<Sphere>(&shape)) {
            area_light_id = sph->area_light_id;
        } else if (auto *tri = std::get_if<Triangle>(&shape)) {
            area_light_id = tri->area_light_id;
        }
        if (area_light_id == -1)
            continue;
        if (auto *area_light = std::get_if<ParsedDiffuseAreaLight>(&scene.lights.at(area_light_id))) {
            int new_light_id = lights.size();
            lights.push_back(DiffuseAreaLight{i, area_light->radiance});
            if (auto *sph = std::get_if<Sphere>(&shape)) {
                sph->area_light_id = new_light_id;
            } else if (auto *tri = std::get_if<Triangle>(&shape)) {
                tri->area_light_id = new_light_id;
            }
        }
    }
}

void build_bvh(Scene& scene) {
    std::vector<BBoxWithID> bboxes(scene.shapes.size());
    for (int i = 0; i < (int)bboxes.size(); i++) {
        if (auto *sph = std::get_if<Sphere>(&scene.shapes[i])) {
            Vector3 p_min = sph->center - sph->radius;
            Vector3 p_max = sph->center + sph->radius;
            bboxes[i] = {BBox{p_min, p_max}, i};
        } else if (auto *tri = std::get_if<Triangle>(&scene.shapes[i])) {
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
    scene.bvh_root_id = construct_bvh(bboxes, scene.bvh_nodes);
}

std::optional<Intersection> bvh_intersect(const Scene &scene, const BVHNode &node, Ray ray) {
    if (node.primitive_id != -1) {
        return std::visit(intersect_op{ray}, scene.shapes[node.primitive_id]);
    }
    const BVHNode &left = scene.bvh_nodes[node.left_node_id];
    const BVHNode &right = scene.bvh_nodes[node.right_node_id];
    std::optional<Intersection> isect_left;
    if (intersect(left.box, ray)) {
        isect_left = bvh_intersect(scene, left, ray);
        if (isect_left) {
            ray.tmax = isect_left->t;
        }
    }
    if (intersect(right.box, ray)) {
        // Since we've already set ray.tfar to the left node
        // if we still hit something on the right, it's closer
        // and we should return that.
        if (auto isect_right = bvh_intersect(scene, right, ray)) {
            return isect_right;
        }
    }
    return isect_left;
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

Vector3 trace_ray(const Scene& scene, const Ray& r, std::mt19937& rng){
    std::optional<Intersection> v_ = scene_intersect(scene, r);
    if(!v_) return scene.background_color;
    Intersection v = *v_;
    Vector3 n = dot(r.dir, v.shading_normal) > 0 ? -v.shading_normal : v.shading_normal;

    if(v.area_light_id != -1) {
        const Light& light = scene.lights.at(v.area_light_id);
        if (auto* l = std::get_if<DiffuseAreaLight>(&light))
            return l->intensity;
    } else if(scene.materials[v.material_id].type == MaterialType::Diffuse){
        Vector3 color = {Real(0), Real(0), Real(0)};
        for(auto& light:scene.lights){
            if (auto* l = std::get_if<PointLight>(&light)){
                Real d = length(l->position - v.pos);
                Vector3 light_dir = normalize(l->position - v.pos);
                Ray shadow_ray = {v.pos, light_dir, c_EPSILON, (1 - c_EPSILON) * d};
                if(!scene_occluded(scene, shadow_ray)){
                    const Vector3& Kd = eval(scene.materials[v.material_id].reflectance, v.uv, scene.textures);
                    color += Kd * max(dot(n, light_dir), Real(0)) * l->intensity / (c_PI * d * d);
                }
            } else if (auto* l = std::get_if<DiffuseAreaLight>(&light)) {
                auto& [light_pos, light_n] = sample_on_light(scene, *l, rng);
                Real d = length(light_pos - v.pos);
                Vector3 light_dir = normalize(light_pos - v.pos);
                Ray shadow_ray = {v.pos, light_dir, c_EPSILON, (1 - c_EPSILON) * d};
                if(!scene_occluded(scene, shadow_ray)){
                    Real pdf_reciprocal = get_area(scene.shapes.at(l->shape_id));
                    const Vector3& Kd = eval(scene.materials[v.material_id].reflectance, v.uv, scene.textures);
                    color += pdf_reciprocal * Kd * max(dot(n, light_dir), Real(0)) * l->intensity * max(dot(-light_n, light_dir), Real(0)) / (c_PI * d * d);
                }
            }
        }
        return color;
    }else if(scene.materials[v.material_id].type == MaterialType::Mirror){
        Ray reflect_ray = {v.pos, r.dir - 2*dot(r.dir, n) * n, c_EPSILON, infinity<Real>()};
        const Vector3& F0 = eval(scene.materials[v.material_id].reflectance, v.uv, scene.textures);
        Vector3 F = F0 + (1 - F0) * pow(1 - dot(n, reflect_ray.dir), 5);
        return F * trace_ray(scene, reflect_ray, rng);
    }else if(scene.materials[v.material_id].type == MaterialType::Plastic){
        // Compute diffuse color
        Vector3 diffuse = {Real(0), Real(0), Real(0)};
        for(auto& light:scene.lights){
            if (auto* l = std::get_if<PointLight>(&light)){
                Real d = length(l->position - v.pos);
                Vector3 light_dir = normalize(l->position - v.pos);
                Ray shadow_ray = {v.pos, light_dir, c_EPSILON, (1 - c_EPSILON) * d};
                if(!scene_occluded(scene, shadow_ray)){
                    const Vector3& Kd = eval(scene.materials[v.material_id].reflectance, v.uv, scene.textures);
                    diffuse += Kd * max(dot(n, light_dir), Real(0)) * l->intensity / (c_PI * d * d);
                }
            } else if (auto* l = std::get_if<DiffuseAreaLight>(&light)) {
                auto& [light_pos, light_n] = sample_on_light(scene, *l, rng);
                Real d = length(light_pos - v.pos);
                Vector3 light_dir = normalize(light_pos - v.pos);
                Ray shadow_ray = {v.pos, light_dir, c_EPSILON, (1 - c_EPSILON) * d};
                if(!scene_occluded(scene, shadow_ray)){
                    Real pdf_reciprocal = get_area(scene.shapes.at(l->shape_id));
                    const Vector3& Kd = eval(scene.materials[v.material_id].reflectance, v.uv, scene.textures);
                    diffuse += pdf_reciprocal * Kd * max(dot(n, light_dir), Real(0)) * l->intensity * max(dot(-light_n, light_dir), Real(0)) / (c_PI * d * d);
                }
            }
        }
        // Compute specular color
        Ray reflect_ray = {v.pos, r.dir - 2*dot(r.dir, n) * n, c_EPSILON, infinity<Real>()};
        Vector3 specular = trace_ray(scene, reflect_ray, rng);
        // Compute blend color
        Real eta = scene.materials[v.material_id].eta;
        Real F0 = pow((eta - 1)/(eta + 1), 2);
        Real F = F0 + (1 - F0) * pow(1 - dot(n, reflect_ray.dir), 5);
        return F * specular + (1 - F) * diffuse;
    }else{
        return scene.background_color;
    }
}