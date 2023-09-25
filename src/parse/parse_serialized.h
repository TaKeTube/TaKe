#pragma once

#include "take.h"
#include "matrix.h"
#include "parse_scene.h"
#include <filesystem>

/// Parse Mitsuba's serialized file format.
TriangleMesh parse_serialized(const fs::path &filename,
                                    int shape_index,
                                    const Matrix4x4 &to_world);
