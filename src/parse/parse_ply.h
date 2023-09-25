#pragma once

#include "take.h"
#include "matrix.h"
#include "parse_scene.h"
#include <filesystem>

/// Parse Stanford PLY files.
TriangleMesh parse_ply(const fs::path &filename, const Matrix4x4 &to_world);
