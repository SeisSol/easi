// Checks the functions of ImpalaJIT that Lua only has with lmathx (LUA_LMATHX): tanh, cosh, sinh,
// erf, erfc and round (rounding halfway cases away from zero, as in C).
#include "easitest.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

int main(int argc, char** argv) {
  assert(argc == 2);

  const std::vector<std::array<double, 3>> points = {
      {0.3, 1.5, 0.75}, {-1.2, 0.1, 2.5}, {2.0, -0.7, -1.5}};
  std::vector<std::pair<int, std::array<double, 3>>> queryPoints;
  for (const auto& point : points) {
    queryPoints.emplace_back(1, point);
  }
  easi::Query query = createQuery<3>(queryPoints);

  const auto values = genericModel(argv[1], query, {"hyperbolic", "special"});

  bool correct = true;
  for (std::size_t p = 0; p < points.size(); ++p) {
    const double x = points[p][0];
    const double y = points[p][1];
    const double z = points[p][2];
    const double hyperbolic = std::tanh(x) - std::cosh(y) / std::sinh(z) - std::tanh(z);
    const double special = std::erf(x) - std::erfc(y) - std::round(2.5) - std::round(-z);
    correct &= equal(values[0][p], hyperbolic, 1e-14 * std::max(1.0, std::fabs(hyperbolic)));
    correct &= equal(values[1][p], special, 1e-14 * std::max(1.0, std::fabs(special)));
  }
  assert(correct);
  return correct ? 0 : 1;
}
