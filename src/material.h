#pragma once
#include "vector.h"
#include "texture.h"

enum class MaterialType {
    Diffuse,
    Mirror,
    Plastic
};

struct Material {
    MaterialType type;
    Texture reflectance;
    Real eta = 1;
};