#ifndef EASITEST_H_
#define EASITEST_H_

#include <cassert>
#include <easi/Query.h>
#include <limits>
#include <string>
#include <vector>

struct ElasticMaterial {
    double lambda, mu, rho;
};

template <unsigned DIM>
easi::Query createQuery(std::vector<std::pair<int, std::array<double, DIM>>> const& values) {
    unsigned numPoints = values.size();
    easi::Query query(numPoints, DIM);
    for (unsigned n = 0; n < numPoints; ++n) {
        query.group(n) = values[n].first;
        for (unsigned d = 0; d < DIM; ++d) {
            query.x(n, d) = values[n].second[d];
        }
    }

    return query;
}

bool equal(double v1, double v2, double eps = std::numeric_limits<double>::epsilon());
std::vector<ElasticMaterial> elasticModel(std::string const& fileName, easi::Query& query);
std::vector<std::vector<double>> genericModel(std::string const& fileName, easi::Query& query,
                                              std::vector<std::string> const& parameters);

extern "C" {
extern void STRESS_STR_DIP_SLIP_AM(double mu_dy, double mu_st, double strike, double dip,
                                   double sigmazz, double cohesion, double R,
                                   double DipSlipFaulting, double s2ratio, double* bii);
}

#endif
