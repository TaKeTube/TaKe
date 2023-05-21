#include "texture.h"

Vector3 eval_texture_op::operator()(const ConstTexture &t) const {
    return t.value;
}

Vector3 eval_texture_op::operator()(const ImageTexture &t) const {
    const Image3& img = pool.image3s.at(t.texture_id);
    Real x = img.width * modulo(t.uscale * uv.x + t.uoffset, Real(1));
    Real y = img.height * modulo(t.vscale * uv.y + t.voffset, Real(1));
    // Bilinear Interpolation
    int x1 = floor(x);
    int x2 = ceil(x) == img.width ? 0 : ceil(x);
    int y1 = floor(y);
    int y2 = ceil(y) == img.height ? 0 : ceil(y);
    Vector3 q11 = img(x1, y1);
    Vector3 q12 = img(x1, y2);
    Vector3 q21 = img(x2, y1);
    Vector3 q22 = img(x2, y2);
    
    Vector3 ret = (q11*(x2-x)*(y2-y)+q21*(x-x1)*(y2-y)+q12*(x2-x)*(y-y1)+q22*(x-x1)*(y-y1))/Real((x2-x1)*(y2-y1));
    // if(std::isnan(ret[0])) {
    //     // std::cout << x2-x1 << " " << y2-y1 << std::endl;
    //     // std::cout << "mod:" << Real(t.vscale * uv.y + t.voffset) << std::endl;
    //     std::cout << "data:" << t.vscale << " " << uv.y << " " << uv.x << " " << t.voffset << std::endl;
    // }
    return (q11*(x2-x)*(y2-y)+q21*(x-x1)*(y2-y)+q12*(x2-x)*(y-y1)+q22*(x-x1)*(y-y1))/Real((x2-x1)*(y2-y1));
    // return img(floor(x), floor(y));
}