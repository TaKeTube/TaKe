#include "hw2.h"
#include "parse_scene.h"
#include "print_scene.h"
#include "timer.h"
#include "parallel.h"
#include "hw2_utility.h"

using namespace hw2;

Image3 hw_2_1(const std::vector<std::string> &params) {
    // Homework 2.1: render a single triangle and outputs
    // its barycentric coordinates.
    // We will use the following camera parameter
    // lookfrom = (0, 0,  0)
    // lookat   = (0, 0, -1)
    // up       = (0, 1,  0)
    // vfov     = 45
    // and we will parse the triangle vertices from params
    // The three vertices are stored in v0, v1, and v2 below.

    std::vector<float> tri_params;
    int spp = 16;
    for (int i = 0; i < (int)params.size(); i++) {
        if (params[i] == "-spp") {
            spp = std::stoi(params[++i]);
        } else {
            tri_params.push_back(std::stof(params[i]));
        }
    }

    if (tri_params.size() < 9) {
        // Not enough parameters to parse the triangle vertices.
        return Image3(0, 0);
    }

    Vector3 p0{tri_params[0], tri_params[1], tri_params[2]};
    Vector3 p1{tri_params[3], tri_params[4], tri_params[5]};
    Vector3 p2{tri_params[6], tri_params[7], tri_params[8]};

    Image3 img(640 /* width */, 480 /* height */);

    // struct Scene {
    //     Camera camera;
    //     int width, height;
    //     std::vector<Shape> shapes;
    //     std::vector<Material> materials;
    //     std::vector<PointLight> lights;
    //     Vector3 background_color;
    //     int samples_per_pixel;
    //     std::vector<TriangleMesh> meshes;
    // };

    Scene scene;
    scene.camera = Camera {
        Vector3{0, 0,  0}, // lookfrom
        Vector3{0, 0, -1}, // lookat
        Vector3{0, 1,  0}, // up
        45                 // vfov
    };
    scene.height = img.height;
    scene.width = img.width;
    scene.materials = std::vector<Material>{
        Material{MaterialType::Diffuse, Vector3{0.75, 0.25, 0.25}}
    };
    scene.lights = std::vector<PointLight>{
        PointLight{Vector3{100, 100, 100}, Vector3{5, 5, -2}}
    };
    scene.background_color = {0.5, 0.5, 0.5};
    scene.samples_per_pixel = spp;
    scene.meshes = std::vector<TriangleMesh>{
        // struct TriangleMesh : public ShapeBase {
        //     std::vector<Vector3> positions;
        //     std::vector<Vector3i> indices;
        //     std::vector<Vector3> normals;
        //     std::vector<Vector2> uvs;
        // };
        TriangleMesh{
            -1, -1,
            {p0, p1, p2},
            {{0, 1, 2}},
            {{}, {}, {}},
            {{}, {}, {}}
        }
    };

    TriangleMesh &mesh = scene.meshes.at(0);
    for (int face_index = 0; face_index < (int)mesh.indices.size(); face_index++) {
        scene.shapes.push_back(Triangle{face_index, &mesh});   
    }

    Camera &cam = scene.camera;

    Real theta = cam.vfov / 180 * c_PI;
    Real h = tan(theta/2);
    Real viewport_height = 2.0 * h;
    Real viewport_width = viewport_height / img.height * img.width;

    Vector3 w = normalize(cam.lookfrom - cam.lookat);
    Vector3 u = normalize(cross(cam.up, w));
    Vector3 v = cross(w, u);

    constexpr int tile_size = 16;
    int num_tiles_x = (img.width + tile_size - 1) / tile_size;
    int num_tiles_y = (img.height + tile_size - 1) / tile_size;
    parallel_for([&](const Vector2i &tile) {
        std::mt19937 rng{std::random_device{}()};
        int x0 = tile[0] * tile_size;
        int x1 = min(x0 + tile_size, img.width);
        int y0 = tile[1] * tile_size;
        int y1 = min(y0 + tile_size, img.height);
        for (int y = y0; y < y1; y++) {
            for (int x = x0; x < x1; x++) {
                Vector3 color = {0, 0, 0};
                for (int i = 0; i < spp; i++){
                    Ray r = {cam.lookfrom, 
                            u * ((x + random_double(rng)) / img.width - Real(0.5)) * viewport_width +
                            v * ((y + random_double(rng)) / img.height - Real(0.5)) * viewport_height -
                            w,
                            epsilon,
                            infinity<Real>()};

                    std::optional<Intersection> v_ = scene_intersect(scene, r);
                    if(!v_) {
                        color += scene.background_color;
                        continue;
                    }
                    Intersection v = *v_;
                    color += Vector3(1-v.uv.x-v.uv.y, v.uv.x, v.uv.y);
                }
                img(x, img.height - y - 1) = color / Real(spp);
            }
        }
    }, Vector2i(num_tiles_x, num_tiles_y));
    return img;
}

Image3 hw_2_2(const std::vector<std::string> &params) {
    // Homework 2.2: render a triangle mesh.
    // We will use the same camera parameter:
    // lookfrom = (0, 0,  0)
    // lookat   = (0, 0, -1)
    // up       = (0, 1,  0)
    // vfov     = 45
    // and we will use a fixed triangle mesh: a tetrahedron!
    int spp = 16;
    for (int i = 0; i < (int)params.size(); i++) {
        if (params[i] == "-spp") {
            spp = std::stoi(params[++i]);
        }
    }

    std::vector<Vector3> positions = {
        Vector3{ 0.0,  0.5, -2.0},
        Vector3{ 0.0, -0.3, -1.0},
        Vector3{ 1.0, -0.5, -3.0},
        Vector3{-1.0, -0.5, -3.0}
    };
    std::vector<Vector3i> indices = {
        Vector3i{0, 1, 2},
        Vector3i{0, 3, 1},
        Vector3i{0, 2, 3},
        Vector3i{1, 2, 3}
    };

    Image3 img(640 /* width */, 480 /* height */);
    
    // struct Scene {
    //     Camera camera;
    //     int width, height;
    //     std::vector<Shape> shapes;
    //     std::vector<Material> materials;
    //     std::vector<PointLight> lights;
    //     Vector3 background_color;
    //     int samples_per_pixel;
    //     std::vector<TriangleMesh> meshes;
    // };
    Scene scene;
    scene.camera = Camera {
        Vector3{0, 0,  0}, // lookfrom
        Vector3{0, 0, -1}, // lookat
        Vector3{0, 1,  0}, // up
        45                 // vfov
    };
    scene.height = img.height;
    scene.width = img.width;
    scene.materials = std::vector<Material>{
        Material{MaterialType::Diffuse, Vector3{0.75, 0.25, 0.25}}
    };
    scene.lights = std::vector<PointLight>{
        PointLight{Vector3{100, 100, 100}, Vector3{5, 5, -2}}
    };
    scene.background_color = {0.5, 0.5, 0.5};
    scene.samples_per_pixel = spp;
    scene.meshes = std::vector<TriangleMesh>{
        // struct TriangleMesh : public ShapeBase {
        //     std::vector<Vector3> positions;
        //     std::vector<Vector3i> indices;
        //     std::vector<Vector3> normals;
        //     std::vector<Vector2> uvs;
        // };
        TriangleMesh{
            -1, -1,
            positions,
            indices,
            {},
            {}
        }
    };
    TriangleMesh &mesh = scene.meshes.at(0);
    for (int face_index = 0; face_index < (int)mesh.indices.size(); face_index++) {
        scene.shapes.push_back(Triangle{face_index, &mesh});   
    }

    Camera &cam = scene.camera;

    Real theta = cam.vfov / 180 * c_PI;
    Real h = tan(theta/2);
    Real viewport_height = 2.0 * h;
    Real viewport_width = viewport_height / img.height * img.width;

    Vector3 w = normalize(cam.lookfrom - cam.lookat);
    Vector3 u = normalize(cross(cam.up, w));
    Vector3 v = cross(w, u);

    constexpr int tile_size = 16;
    int num_tiles_x = (img.width + tile_size - 1) / tile_size;
    int num_tiles_y = (img.height + tile_size - 1) / tile_size;
    parallel_for([&](const Vector2i &tile) {
        std::mt19937 rng{std::random_device{}()};
        int x0 = tile[0] * tile_size;
        int x1 = min(x0 + tile_size, img.width);
        int y0 = tile[1] * tile_size;
        int y1 = min(y0 + tile_size, img.height);
        for (int y = y0; y < y1; y++) {
            for (int x = x0; x < x1; x++) {
                Vector3 color = {0, 0, 0};
                for (int i = 0; i < spp; i++){
                    Ray r = {cam.lookfrom, 
                            u * ((x + random_double(rng)) / img.width - Real(0.5)) * viewport_width +
                            v * ((y + random_double(rng)) / img.height - Real(0.5)) * viewport_height -
                            w,
                            epsilon,
                            infinity<Real>()};

                    std::optional<Intersection> v_ = scene_intersect(scene, r);
                    if(!v_) {
                        color += scene.background_color;
                        continue;
                    }
                    Intersection v = *v_;
                    color += Vector3(1-v.uv.x-v.uv.y, v.uv.x, v.uv.y);
                }
                img(x, img.height - y - 1) = color / Real(spp);
            }
        }
    }, Vector2i(num_tiles_x, num_tiles_y));

    return img;
}

Image3 hw_2_3(const std::vector<std::string> &params) {
    // Homework 2.3: render a scene file provided by our parser.
    if (params.size() < 1) {
        return Image3(0, 0);
    }

    Timer timer;
    tick(timer);
    ParsedScene pscene = parse_scene(params[0]);
    std::cout << "Scene parsing done. Took " << tick(timer) << " seconds." << std::endl;
    std::cout << pscene << std::endl;

    Scene scene(pscene);

    Image3 img(scene.width, scene.height);

    Camera &cam = scene.camera;

    Real theta = cam.vfov / 180 * c_PI;
    Real h = tan(theta/2);
    Real viewport_height = 2.0 * h;
    Real viewport_width = viewport_height / img.height * img.width;

    Vector3 w = normalize(cam.lookfrom - cam.lookat);
    Vector3 u = normalize(cross(cam.up, w));
    Vector3 v = cross(w, u);

    constexpr int tile_size = 16;
    int num_tiles_x = (img.width + tile_size - 1) / tile_size;
    int num_tiles_y = (img.height + tile_size - 1) / tile_size;
    parallel_for([&](const Vector2i &tile) {
        std::mt19937 rng{std::random_device{}()};
        int x0 = tile[0] * tile_size;
        int x1 = min(x0 + tile_size, img.width);
        int y0 = tile[1] * tile_size;
        int y1 = min(y0 + tile_size, img.height);
        for (int y = y0; y < y1; y++) {
            for (int x = x0; x < x1; x++) {
                Vector3 color = {0, 0, 0};
                for (int i = 0; i < scene.samples_per_pixel; i++){
                    Ray r = {cam.lookfrom, 
                            u * ((x + random_double(rng)) / img.width - Real(0.5)) * viewport_width +
                            v * ((y + random_double(rng)) / img.height - Real(0.5)) * viewport_height -
                            w,
                            epsilon,
                            infinity<Real>()};
                    color += trace_ray(scene, r);
                }
                img(x, img.height - y - 1) = color / Real(scene.samples_per_pixel);
            }
        }
    }, Vector2i(num_tiles_x, num_tiles_y));

    return img;
}

Image3 hw_2_4(const std::vector<std::string> &params) {
    // Homework 2.4: render the AABBs of the scene.
    if (params.size() < 1) {
        return Image3(0, 0);
    }

    Timer timer;
    tick(timer);
    ParsedScene scene = parse_scene(params[0]);
    std::cout << "Scene parsing done. Took " << tick(timer) << " seconds." << std::endl;
    UNUSED(scene);

    return Image3(0, 0);
}

Image3 hw_2_5(const std::vector<std::string> &params) {
    // Homework 2.5: rendering with BVHs
    if (params.size() < 1) {
        return Image3(0, 0);
    }

    Timer timer;
    tick(timer);
    ParsedScene scene = parse_scene(params[0]);
    std::cout << "Scene parsing done. Took " << tick(timer) << " seconds." << std::endl;
    UNUSED(scene);

    return Image3(0, 0);
}
