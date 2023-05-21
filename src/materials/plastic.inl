std::optional<SampleRecord> sample_bsdf_op::operator()(const Plastic &m) const {
    return {};
}

Vector3 eval_material_op::operator()(const Plastic &m) const {
    return {};
}