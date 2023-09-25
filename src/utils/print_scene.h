#pragma once

#include "parse/parse_scene.h"
#include <iostream>

std::ostream& operator<<(std::ostream &os, const Texture &color);
std::ostream& operator<<(std::ostream &os, const Camera &camera);
std::ostream& operator<<(std::ostream &os, const Material &material);
std::ostream& operator<<(std::ostream &os, const Light &light);
std::ostream& operator<<(std::ostream &os, const Shape &shape);
std::ostream& operator<<(std::ostream &os, const Scene &scene);
