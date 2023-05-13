#pragma once
#include "vector.h"
#include "texture.h"

enum class MaterialType {
    Diffuse,
    Mirror
};

struct Material {
    MaterialType type;
    Texture reflectance;
};