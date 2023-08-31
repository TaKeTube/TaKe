cmake_minimum_required(VERSION 3.10)

project(take)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE RelWithDebInfo)

# if(MSVC)
#   add_compile_options(/Wall)
# else()
#   add_compile_options(-Wall)
# endif()

include_directories(${CMAKE_SOURCE_DIR}/src)

set(SRCS src/3rdparty/miniz.h
         src/3rdparty/miniz.c
         src/3rdparty/pugiconfig.hpp
         src/3rdparty/pugixml.hpp
         src/3rdparty/pugixml.cpp
         src/3rdparty/stb_image.h
         src/3rdparty/tinyexr.h
         src/3rdparty/tinyply.h
         src/compute_normals.h
         src/flexception.h
         src/render.h
         src/render.cpp
         src/image.h
         src/matrix.h
         src/parallel.h
         src/parse_obj.h
         src/parse_ply.h
         src/parse_scene.h
         src/parse_serialized.h
         src/print_scene.h
         src/take.h
         src/transform.h
         src/vector.h
         src/compute_normals.cpp
         src/image.cpp
         src/main.cpp
         src/parallel.cpp
         src/parse_obj.cpp
         src/parse_ply.cpp
         src/parse_scene.cpp
         src/parse_serialized.cpp
         src/print_scene.cpp
         src/transform.cpp
         src/bbox.h
         src/bvh.h         
         src/bvh.cpp
         src/camera.h
         src/camera.cpp
         src/intersection.h
         src/light.h
         src/light.cpp
         src/material.h
         src/material.cpp
         src/ray.h
         src/scene.h
         src/scene.cpp
         src/shape.h
         src/shape.cpp
         src/texture.h
         src/texture.cpp
         src/materials/diffuse.inl
         src/materials/mirror.inl
         src/materials/plastic.inl
         src/materials/phong.inl
         src/materials/blinn_phong.inl
         src/materials/blinn_phong_microfacet.inl
         )

add_library(take_lib STATIC ${SRCS})
add_executable(take src/main.cpp)
target_link_libraries(take take_lib)
