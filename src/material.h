#pragma once
#include "vector.h"

enum class MaterialType {
Diffuse,
Mirror
};

struct Material {
    MaterialType type;
    Vector3 color;
};