Real light_power_op::operator()(const PointLight &l) const {
    return luminance(l.intensity);
}

PointAndNormal sample_on_light_op::operator()(const PointLight &l) const{
    return {l.position, Vector3{0, 0, 0}};
}

Real sample_on_light_pdf_op::operator()(const PointLight &l) const {
    return Real(1);
}

Vector3 emission_op::operator()(const PointLight &l) const {
    return l.intensity;
}

void init_sample_dist_op::operator()(PointLight &l) const {
}