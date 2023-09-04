std::optional<SampleRecord> sample_bsdf_op::operator()(const DisneyDiffuse &m) const {
    if (dot(v.geo_normal, dir_in) < 0) {
        return {};
    }
    Vector3 n = dot(dir_in, v.shading_normal) < 0 ? -v.shading_normal : v.shading_normal;
    SampleRecord record;
    record.dir_out = to_world(n, sample_hemisphere_cos(rng));
    if (dot(v.geo_normal, record.dir_out) < 0) 
        record.pdf = Real(0);
    else
        record.pdf = fmax(dot(n, record.dir_out), Real(0)) / c_PI;
    return record;
}

Real sample_bsdf_pdf_op::operator()(const DisneyDiffuse &m) const {
    if (dot(v.geo_normal, dir_out) < 0) 
        return Real(0);
    Vector3 n = dot(dir_in, v.shading_normal) < 0 ? -v.shading_normal : v.shading_normal;
    return fmax(dot(n, dir_out), Real(0)) / c_PI;
}

Vector3 eval_material_op::operator()(const DisneyDiffuse &m) const {
    if (dot(v.geo_normal, dir_in) < 0 || dot(v.geo_normal, record.dir_out) < 0)
        return {Real(0), Real(0), Real(0)};
    
    Vector3 n = dot(dir_in, v.shading_normal) < 0 ? -v.shading_normal : v.shading_normal;
    const Vector3 &dir_out = record.dir_out;
    Vector3 h = normalize(dir_in + dir_out);
    Real hdout = dot(h, dir_out);
    Real ndout = dot(n, dir_out);
    Real ndin = dot(n, dir_in);
    const Vector3& Kd = eval(m.reflectance, v.uv, texture_pool);

    auto F = [](const Vector3& w, const Vector3&n, Real FF){
        return 1 + (FF - 1) * pow(1 - dot(n, w), Real(5));
    };

    // Compute base diffuse
    Real F_D90 = Real(0.5) + 2 * m.roughness * hdout * hdout;
    Vector3 f_base_diffuse = Kd * c_INVPI * F(dir_in, n, F_D90) * F(dir_out, n, F_D90) * ndout;

    // Compute subsurface
    Real F_SS90 = m.roughness * hdout * hdout;
    Vector3 f_subsurface = Real(1.25) * Kd * c_INVPI * (F(dir_in, n, F_SS90) * F(dir_out, n, F_SS90) * (1 / (abs(ndin) + abs(ndout)) - Real(0.5)) + Real(0.5)) * ndout;

    return (1 - m.subsurface) * f_base_diffuse + m.subsurface * f_subsurface;
}