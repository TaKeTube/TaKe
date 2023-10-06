Real light_power_op::operator()(const Envmap &l) const {
    return c_PI * scene.bounds.radius * scene.bounds.radius *
           l.dist.marginal.fint / (l.dist.marginal.pmf.size() * l.dist.conditional[0].pmf.size());
}

PointAndNormal sample_on_light_op::operator()(const Envmap &l) const{
    Vector2 uv = sample_continuous(l.dist, rng);
    Real phi = uv.x * c_TWOPI;
    Real theta = uv.y * c_PI;
    // y-axis is up direction
    Vector3 local_dir = {sin(theta) * sin(phi),
                         cos(theta),
                         -sin(theta) * cos(phi)};
    Vector3 world_dir = xform_vector(l.to_world, local_dir);
    return {Vector3{0, 0, 0}, -local_dir};
}

Real sample_on_light_pdf_op::operator()(const Envmap &l) const {
    Vector3 world_dir = -light_point.normal;
    Vector3 local_dir = xform_vector(l.to_local, world_dir);
    // y-axis is up direction
    Vector2 uv{atan2(local_dir[0], -local_dir[2]) * c_INVTWOPI,
               acos(local_dir[1]) * c_INVPI};
    // atan2 returns -pi to pi, we map [-pi, 0] to [pi, 2pi]
    // TODO PBRT does this, but why?
    if (uv[0] < 0) {
        uv[0] += 1;
    }
    Real cos_theta = local_dir.y;
    Real sin_theta = sqrt(std::clamp(1 - cos_theta * cos_theta, Real(0), Real(1)));
    if (sin_theta <= 0) {
        return 0;
    }
    return get_dist2D_pdf(l.dist, uv) / (2 * c_PI * c_PI * sin_theta);
}

Vector3 emission_op::operator()(const Envmap &l) const {
    Vector3 world_dir = -light_point.normal;
    Vector3 local_dir = xform_vector(l.to_local, world_dir);
    // y-axis is up direction
    Vector2 uv{atan2(local_dir[0], -local_dir[2]) * c_INVTWOPI,
               acos(local_dir[1]) * c_INVPI};
    if (uv[0] < 0) {
        uv[0] += 1;
    }
    return eval(l.intensity, uv, scene.textures) * l.scale;
}