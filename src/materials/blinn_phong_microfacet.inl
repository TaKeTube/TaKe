std::optional<SampleRecord> sample_bsdf_op::operator()(const BlinnPhongMicrofacet &m) const {
    if (dot(v.geo_normal, dir_in) < 0) {
        return {};
    }
    Vector3 n = dot(dir_in, v.shading_normal) < 0 ? -v.shading_normal : v.shading_normal;

    return {};
}

Vector3 eval_material_op::operator()(const BlinnPhongMicrofacet &m) const{
    if (dot(v.geo_normal, dir_in) < 0 || dot(v.geo_normal, record.dir_out) < 0)
        return {Real(0), Real(0), Real(0)};
    Vector3 n = dot(dir_in, v.shading_normal) < 0 ? -v.shading_normal : v.shading_normal;

    return {Real(0), Real(0), Real(0)};
}