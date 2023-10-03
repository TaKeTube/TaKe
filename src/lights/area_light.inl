Real light_power_op::operator()(const AreaLight &l) const {
    return luminance(l.intensity) * get_area(scene.shapes[l.shape_id], scene.meshes);
}

PointAndNormal sample_on_light_op::operator()(const AreaLight &l) const{
    return std::visit(sample_on_shape_op{scene.meshes, ref_pos, rng}, scene.shapes.at(l.shape_id));
}

Real sample_on_light_pdf_op::operator()(const AreaLight &l) const {
    return get_shape_pdf(scene.shapes[l.shape_id], ref_pos, light_point, scene.meshes);
}

Vector3 emission_op::operator()(const AreaLight &l) const {
    if (dot(light_point.normal, view_dir) <= 0) {
        return {0, 0, 0};
    }
    return l.intensity;
}