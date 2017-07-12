#ifndef EASITEST_H_
#define EASITEST_H_

#include <cassert>
#include <vector>
#include <string>
#include <limits>
#include <easi/Query.h>

struct ElasticMaterial {
  double lambda, mu, rho;
};

template<unsigned DIM>
easi::Query createQuery(std::vector<std::pair<int, std::array<double, DIM>>> const& values) {
  unsigned numPoints = values.size();
  easi::Query query(numPoints, DIM);
  for (unsigned n = 0; n < numPoints; ++n) {
    query.group(n) = values[n].first;
    for (unsigned d = 0; d < DIM; ++d) {
      query.x(n,d) = values[n].second[d];
    }
  }
  
  return query;
}

bool equal(double v1, double v2, double eps = std::numeric_limits<double>::epsilon());
std::vector<ElasticMaterial> elasticModel(std::string const& fileName, easi::Query& query);

#endif
