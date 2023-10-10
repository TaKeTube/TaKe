#include "distribution.h"

Distribution1D make_dist1D(const std::vector<Real> &f) {
    std::vector<Real> pmf = f;
    std::vector<Real> cdf(pmf.size());
    assert(pmf[0] >= 0);
    cdf[0] = pmf[0];
    for(int i = 1; i < static_cast<int>(pmf.size()); ++i){
        assert(pmf[i] >= 0);
        cdf[i] = cdf[i-1] + pmf[i];
    }
    Real s = cdf.back();
    if(s > 0){
        for(int i = 0; i < static_cast<int>(pmf.size()); ++i){
            pmf[i] /= s;
            cdf[i] /= s;
        }
    }else{
        Real p = Real(1) / Real(pmf.size());
        for(int i = 0; i < static_cast<int>(pmf.size()); ++i){
            pmf[i] = p;
            cdf[i] = (i+1) * p;
        }
    }
    return Distribution1D{std::move(pmf), std::move(cdf), s};
}

int sample_discrete(const Distribution1D& dist, std::mt19937& rng){
    Real p = random_real(rng);
    auto it = std::upper_bound(dist.cdf.begin(), dist.cdf.end(), p);
    return std::distance(dist.cdf.begin(), it);
}

Real sample_continuous(const Distribution1D& dist, std::mt19937& rng){
    Real p = random_real(rng);
    auto it = std::upper_bound(dist.cdf.begin(), dist.cdf.end(), p);
    int offset = std::distance(dist.cdf.begin(), it);
    Real cdf_left = (offset == 0) ? 0 : dist.cdf[offset-1];
    Real du = p - cdf_left;
    // TODO Just a reminder 
    // Seems cdf[offset] == cdf[offset - 1] cannot happen so no need an extra if statement
    du /= dist.cdf[offset] - cdf_left;
    return (Real(offset) + du) / Real(dist.pmf.size());
}

Real get_dist1D_pmf(const Distribution1D &dist, const int idx) {
    assert(idx >= 0 && idx < static_cast<int>(dist.pmf.size()));
    return dist.pmf[idx];
}

Real get_dist1D_pdf(const Distribution1D &dist, const Real u) {
    assert(u >= 0 && u < 1);
    int idx = static_cast<int>(u * Real(dist.pmf.size()));
    return dist.pmf[idx] * Real(dist.pmf.size());
}

Distribution2D make_dist2D(const std::vector<std::vector<Real>> &f){
    int w = static_cast<int>(f[0].size());
    int h = static_cast<int>(f.size());

    std::vector<Distribution1D> conditional;
    conditional.reserve(w);
    for(auto& ff : f)
        conditional.push_back(std::move(make_dist1D(ff)));
    
    std::vector<Real> marginal_f;
    marginal_f.reserve(h);
    for(auto& dist : conditional)
        marginal_f.push_back(dist.fint);
    
    Distribution1D marginal = make_dist1D(marginal_f);
    return Distribution2D{std::move(conditional), std::move(marginal)};
}

Vector2i sample_discrete(const Distribution2D& dist, std::mt19937& rng){
    int y = sample_discrete(dist.marginal, rng);
    int x = sample_discrete(dist.conditional[y], rng);
    return Vector2i{x, y};
}

Vector2 sample_continuous(const Distribution2D& dist, std::mt19937& rng){
    Real v = sample_continuous(dist.marginal, rng);
    Real u = sample_continuous(dist.conditional[static_cast<int>(v * dist.marginal.pmf.size())], rng);
    return Vector2{u, v};
}

Real get_dist2D_pmf(const Distribution2D &dist, const Vector2i& idx){
    auto [x, y] = idx;
    int w = static_cast<int>(dist.conditional[0].pmf.size()), h = static_cast<int>(dist.marginal.pmf.size());
    assert(y >= 0 && y < h && x >= 0 && x < w);
    return dist.marginal.pmf[y] * dist.conditional[y].pmf[x]; 
}

Real get_dist2D_pdf(const Distribution2D &dist, const Vector2& uv){
    auto [u, v] = uv;
    assert(u >= 0 && u < 1 && v >= 0 && v < 1);
    int h = static_cast<int>(dist.marginal.pmf.size());
    int y = static_cast<int>(v * h);
    Real pdf_y = get_dist1D_pdf(dist.marginal, v);
    Real pdf_x = get_dist1D_pdf(dist.conditional[y], u);
    return pdf_x * pdf_y;
}