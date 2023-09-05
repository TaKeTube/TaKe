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
    auto get_texture = [&](const ParsedImageTexture *color){
        std::string texture_name = color->filename.string();
        int texture_id;
        auto it = textures.image3s_map.find(texture_name);
        if (it != textures.image3s_map.end()) {
            texture_id = textures.image3s_map.at(texture_name);
        } else {
            texture_id = static_cast<int>(textures.image3s.size());
            textures.image3s_map.emplace(texture_name, texture_id);
            textures.image3s.push_back(imread3(color->filename));
        }
        return ImageTexture{
            texture_id,
            color->uscale, color->vscale,
            color->uoffset, color->voffset,
        };
    };
    
    for (const ParsedMaterial &parsed_mat : scene.materials)
    {
        if (auto *diffuse = std::get_if<ParsedDiffuse>(&parsed_mat))
        {
            if (auto *color = std::get_if<Vector3>(&diffuse->reflectance)) {
                ConstTexture texture = {*color};
                materials.push_back(Diffuse{texture});
            } else if (auto *color = std::get_if<ParsedImageTexture>(&diffuse->reflectance)) {
                ImageTexture texture = get_texture(color);
                materials.push_back(Diffuse{texture});
            }
        }
        else if (auto *mirror = std::get_if<ParsedMirror>(&parsed_mat))
        {
            if (auto *color = std::get_if<Vector3>(&mirror->reflectance)) {
                ConstTexture texture = {*color};
                materials.push_back(Mirror{texture});
            } else if (auto *color = std::get_if<ParsedImageTexture>(&mirror->reflectance)) {
                ImageTexture texture = get_texture(color);
                materials.push_back(Mirror{texture});
            }
        }
        else if (auto *plastic = std::get_if<ParsedPlastic>(&parsed_mat))
        {
            if (auto *color = std::get_if<Vector3>(&plastic->reflectance)) {
                ConstTexture texture = {*color};
                materials.push_back(Plastic{texture, plastic->eta});
            } else if (auto *color = std::get_if<ParsedImageTexture>(&plastic->reflectance)) {
                ImageTexture texture = get_texture(color);
                materials.push_back(Plastic{texture, plastic->eta});
            }
        }
        else if (auto *phong = std::get_if<ParsedPhong>(&parsed_mat))
        {
            if (auto *color = std::get_if<Vector3>(&phong->reflectance)) {
                ConstTexture texture = {*color};
                materials.push_back(Phong{texture, phong->exponent});
            } else if (auto *color = std::get_if<ParsedImageTexture>(&phong->reflectance)) {
                ImageTexture texture = get_texture(color);
                materials.push_back(Phong{texture, phong->exponent});
            }
        }
        else if (auto *blinn_phong = std::get_if<ParsedBlinnPhong>(&parsed_mat))
        {
            if (auto *color = std::get_if<Vector3>(&blinn_phong->reflectance)) {
                ConstTexture texture = {*color};
                materials.push_back(BlinnPhong{texture, blinn_phong->exponent});
            } else if (auto *color = std::get_if<ParsedImageTexture>(&blinn_phong->reflectance)) {
                ImageTexture texture = get_texture(color);
                materials.push_back(BlinnPhong{texture, blinn_phong->exponent});
            }
        }
        else if (auto *blinn_phong_microfacet = std::get_if<ParsedBlinnPhongMicrofacet>(&parsed_mat))
        {
            if (auto *color = std::get_if<Vector3>(&blinn_phong_microfacet->reflectance)) {
                ConstTexture texture = {*color};
                materials.push_back(BlinnPhongMicrofacet{texture, blinn_phong_microfacet->exponent});
            } else if (auto *color = std::get_if<ParsedImageTexture>(&blinn_phong_microfacet->reflectance)) {
                ImageTexture texture = get_texture(color);
                materials.push_back(BlinnPhongMicrofacet{texture, blinn_phong_microfacet->exponent});
            }
        }
        else if (auto *disney_diffuse = std::get_if<ParsedDisneyDiffuse>(&parsed_mat))
        {
            if (auto *color = std::get_if<Vector3>(&disney_diffuse->reflectance)) {
                ConstTexture texture = {*color};
                materials.push_back(DisneyDiffuse{texture, disney_diffuse->roughness, disney_diffuse->subsurface});
            } else if (auto *color = std::get_if<ParsedImageTexture>(&disney_diffuse->reflectance)) {
                ImageTexture texture = get_texture(color);
                materials.push_back(DisneyDiffuse{texture, disney_diffuse->roughness, disney_diffuse->subsurface});
            }
        }
        else if (auto *disney_metal = std::get_if<ParsedDisneyMetal>(&parsed_mat))
        {
            if (auto *color = std::get_if<Vector3>(&disney_metal->reflectance)) {
                ConstTexture texture = {*color};
                materials.push_back(DisneyMetal{texture, disney_metal->roughness, disney_metal->anisotropic});
            } else if (auto *color = std::get_if<ParsedImageTexture>(&disney_metal->reflectance)) {
                ImageTexture texture = get_texture(color);
                materials.push_back(DisneyMetal{texture, disney_metal->roughness, disney_metal->anisotropic});
            }
        }
        else if (auto *disney_glass = std::get_if<ParsedDisneyGlass>(&parsed_mat))
        {
            if (auto *color = std::get_if<Vector3>(&disney_glass->reflectance)) {
                ConstTexture texture = {*color};
                materials.push_back(DisneyGlass{texture, disney_glass->roughness, disney_glass->anisotropic, disney_glass->eta});
            } else if (auto *color = std::get_if<ParsedImageTexture>(&disney_glass->reflectance)) {
                ImageTexture texture = get_texture(color);
                materials.push_back(DisneyGlass{texture, disney_glass->roughness, disney_glass->anisotropic, disney_glass->eta});
            }
        }
        else if (auto *disney_clearcoat = std::get_if<ParsedDisneyClearcoat>(&parsed_mat))
        {
            materials.push_back(DisneyClearcoat{disney_clearcoat->clearcoat_gloss});
        }
        else if (auto *disney_sheen = std::get_if<ParsedDisneySheen>(&parsed_mat))
        {
            if (auto *color = std::get_if<Vector3>(&disney_sheen->reflectance)) {
                ConstTexture texture = {*color};
                materials.push_back(DisneyGlass{texture, disney_sheen->sheen_tint});
            } else if (auto *color = std::get_if<ParsedImageTexture>(&disney_sheen->reflectance)) {
                ImageTexture texture = get_texture(color);
                materials.push_back(DisneyGlass{texture, disney_sheen->sheen_tint});
            }
        }
        else if (auto *disney_bsdf = std::get_if<ParsedDisneyBSDF>(&parsed_mat))
        {
            if (auto *color = std::get_if<Vector3>(&disney_bsdf->reflectance)) {
                ConstTexture texture = {*color};
                materials.push_back(DisneyBSDF{
                    texture,
                    disney_bsdf->specular_transmission,
                    disney_bsdf->metallic,
                    disney_bsdf->subsurface,
                    disney_bsdf->specular,
                    disney_bsdf->roughness,
                    disney_bsdf->specular_tint,
                    disney_bsdf->anisotropic,
                    disney_bsdf->sheen,
                    disney_bsdf->sheen_tint,
                    disney_bsdf->clearcoat,
                    disney_bsdf->clearcoat_gloss,
                    disney_bsdf->eta
                });
            } else if (auto *color = std::get_if<ParsedImageTexture>(&disney_bsdf->reflectance)) {
                ImageTexture texture = get_texture(color);
                materials.push_back(DisneyBSDF{
                    texture,
                    disney_bsdf->specular_transmission,
                    disney_bsdf->metallic,
                    disney_bsdf->subsurface,
                    disney_bsdf->specular,
                    disney_bsdf->roughness,
                    disney_bsdf->specular_tint,
                    disney_bsdf->anisotropic,
                    disney_bsdf->sheen,
                    disney_bsdf->sheen_tint,
                    disney_bsdf->clearcoat,
                    disney_bsdf->clearcoat_gloss,
                    disney_bsdf->eta
                });
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
            int new_light_id = static_cast<int>(lights.size());
            lights.push_back(DiffuseAreaLight{i, area_light->radiance});
            if (auto *sph = std::get_if<Sphere>(&shape)) {
                sph->area_light_id = new_light_id;
            } else if (auto *tri = std::get_if<Triangle>(&shape)) {
                tri->area_light_id = new_light_id;
            }
        }
    }

    std::vector<Real> power(lights.size());
    for (int i = 0; i < (int)lights.size(); i++) {
        power[i] = light_power(*this, lights[i]);
    }
    std::vector<Real>& pmf = power;
    std::vector<Real> cdf(power.size() + 1);
    cdf[0] = 0;
    for (int i = 0; i < (int)power.size(); i++) {
        assert(pmf[i] >= 0);
        cdf[i + 1] = cdf[i] + pmf[i];
    }
    Real total = cdf.back();
    if (total > 0) {
        for (int i = 0; i < (int)pmf.size(); i++) {
            pmf[i] /= total;
            cdf[i] /= total;
        }
    } else {
        for (int i = 0; i < (int)pmf.size(); i++) {
            pmf[i] = Real(1) / Real(pmf.size());
            cdf[i] = Real(i) / Real(pmf.size());
        }
        cdf.back() = 1;
    }
    lights_power_pmf = std::move(pmf);
    lights_power_cdf = std::move(cdf);
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

std::optional<Intersection> scene_intersect(const Scene& scene, const Ray& r){
    if(!scene.bvh_nodes.empty()){
        return bvh_intersect(scene.bvh_root_id, scene.bvh_nodes, scene.shapes, r);
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
        std::optional<Intersection> v_ = bvh_intersect(scene.bvh_root_id, scene.bvh_nodes, scene.shapes, r);
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