#pragma once

#include "take.h"
#include "matrix.h"
#include "parse_scene.h"
#include <filesystem>

/// Parse Wavefront obj files. Currently only supports triangles and quads.
/// Throw errors if encountered general polygons.
TriangleMesh parse_obj(const fs::path &filename, const Matrix4x4 &to_world);
