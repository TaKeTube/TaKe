#pragma once

// CMake insert NDEBUG when building with RelWithDebInfo
// This is an ugly hack to undo that...
#undef NDEBUG

#include <cassert>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <limits>
#include <algorithm>
#include <random>

// for suppressing unused warnings
#define UNUSED(x) (void)(x)

// We use double for most of our computation.
// Rendering is usually done in single precision Reals.
// However, torrey is an educational renderer with does not
// put emphasis on the absolute performance. 
// We choose double so that we do not need to worry about
// numerical accuracy as much when we render.
// Switching to floating point computation is easy --
// just set Real = float.
using Real = double;
// using Real = float;

const Real c_EPSILON = Real(1e-7);
// const Real c_EPSILON = Real(1e-4);

// Lots of PIs!
constexpr Real c_PI = Real(3.14159265358979323846);
constexpr Real c_INVPI = Real(1.0) / c_PI;
constexpr Real c_TWOPI = Real(2.0) * c_PI;
constexpr Real c_INVTWOPI = Real(1.0) / c_TWOPI;
constexpr Real c_FOURPI = Real(4.0) * c_PI;
constexpr Real c_INVFOURPI = Real(1.0) / c_FOURPI;
constexpr Real c_PIOVERTWO = Real(0.5) * c_PI;
constexpr Real c_PIOVERFOUR = Real(0.25) * c_PI;

template <typename T>
inline T infinity() {
    return std::numeric_limits<T>::infinity();
}

namespace fs = std::filesystem;

inline std::string to_lowercase(const std::string &s) {
    std::string out = s;
    std::transform(s.begin(), s.end(), out.begin(), ::tolower);
    return out;
}

inline int modulo(int a, int b) {
    auto r = a % b;
    return (r < 0) ? r+b : r;
}

inline float modulo(float a, float b) {
    float r = ::fmodf(a, b);
    return (r < 0.0f) ? r+b : r;
}

inline double modulo(double a, double b) {
    double r = ::fmod(a, b);
    return (r < 0.0) ? r+b : r;
}

template <typename T>
inline T max(const T &a, const T &b) {
    return a > b ? a : b;
}

template <typename T>
inline T min(const T &a, const T &b) {
    return a < b ? a : b;
}

inline Real radians(const Real deg) {
    return (c_PI / Real(180)) * deg;
}

inline Real degrees(const Real rad) {
    return (Real(180) / c_PI) * rad;
}

inline Real random_real(std::mt19937 &rng) {
    return std::uniform_real_distribution<Real>{0.0, 1.0}(rng);
}

inline int random_int(int min, int max, std::mt19937 &rng) {
    return static_cast<int>(min + (max - min) * random_real(rng));
}