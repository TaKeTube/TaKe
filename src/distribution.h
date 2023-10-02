#pragma once

#include "take.h"
#include "vector.h"
#include <vector>

struct Distribution1D {
    std::vector<Real> pmf;
    std::vector<Real> cdf;
    Real fint; // Integral of discrete f[i]
};

struct Distribution2D {
    std::vector<Distribution1D> conditional;
    Distribution1D marginal;
};

Distribution1D make_dist_1d(const std::vector<Real> &f);
int sample_discrete(const Distribution1D& dist, std::mt19937& rng);
Real sample_continuous(const Distribution1D& dist, std::mt19937& rng);
inline Real get_dist1d_pmf(const Distribution1D &dist, const int idx);
inline Real get_dist1d_pdf(const Distribution1D &dist, const Real u);

Distribution2D make_dist_2d(const std::vector<Real> &f);
Vector2i sample_discrete(const Distribution2D& dist, std::mt19937& rng);
Vector2 sample_continuous(const Distribution2D& dist, std::mt19937& rng);
inline Real get_dist2d_pmf(const Distribution2D &dist, const Vector2i& idx);
inline Real get_dist2d_pdf(const Distribution2D &dist, const Vector2& uv);