// Checks that !FunctionMap keeps the semantics of ImpalaJIT (which easi used for it up to 1.4)
// now that it is translated to Lua: left-associative - and /, assignments to inputs, local
// variables, conditions, keywords within names, and double precision throughout.
#include "easitest.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

struct Expected {
  std::string name;
  double (*value)(double x, double y, double z);
};

double sub_chain(double x, double y, double z) { return x - y - z; }
double add_sub(double x, double y, double z) { return x - y + z; }
double div_chain(double x, double y, double z) { return x / y / z; }
double div_mul(double x, double y, double z) { return x / y * z; }
double mixed(double x, double y, double z) { return 2 * x - y * 3 - z / 4 + 1; }
double unary(double x, double y, double z) { return -x * -y - -z + +x; }
double input_assign(double x, double, double) { return x - 10; }
double locals(double, double, double z) { return -z - 1; }
double branch(double x, double y, double z) { return (x + 1) * 2 > y ? x - y - z : z - y - x; }
double nested(double x, double y, double) { return x > 0 ? (y > 0 ? 1 : 2) : 3; }
double and_or(double x, double y, double z) { return (x > 0 && y > 0) || z < 0 ? 1 : 0; }
double bracketed(double x, double y, double z) {
  return (x > 0 || y > 0) && (z - 1) * 2 < 4 ? 1 : 0;
}
double keywords(double x, double, double) { return 2 * x - 3; }
double statement(double x, double, double) { return x; }
double large(double, double, double) { return 3037000500.0 * 3037000500.0 * 2.0; }
double functions(double x, double y, double z) {
  return std::pow(x, 2) - std::fmax(y, z) - std::fmin(y, z) - std::fabs(-x) - std::atan2(y, x) -
         std::log10(100.0) - std::floor(-0.5) - std::ceil(0.5) - std::sqrt(16.0) - std::exp(1.0) -
         std::log(2.0);
}

} // namespace

int main(int argc, char** argv) {
  assert(argc == 2);

  const std::vector<std::array<double, 3>> points = {
      {3.5, 2.0, 0.25}, {-1.0, 4.0, 2.0}, {10.0, -3.0, 7.0}, {0.5, -0.5, -2.0}};
  std::vector<std::pair<int, std::array<double, 3>>> queryPoints;
  for (const auto& point : points) {
    queryPoints.emplace_back(1, point);
  }
  easi::Query query = createQuery<3>(queryPoints);

  const std::vector<Expected> expected = {{"sub_chain", sub_chain},
                                          {"add_sub", add_sub},
                                          {"div_chain", div_chain},
                                          {"div_mul", div_mul},
                                          {"mixed", mixed},
                                          {"unary", unary},
                                          {"input_assign", input_assign},
                                          {"locals", locals},
                                          {"branch", branch},
                                          {"nested", nested},
                                          {"and_or", and_or},
                                          {"bracketed", bracketed},
                                          {"keywords", keywords},
                                          {"statement", statement},
                                          {"large", large},
                                          {"functions", functions}};
  std::vector<std::string> names;
  for (const auto& entry : expected) {
    names.push_back(entry.name);
  }

  const auto values = genericModel(argv[1], query, names);

  bool correct = true;
  for (std::size_t i = 0; i < expected.size(); ++i) {
    for (std::size_t p = 0; p < points.size(); ++p) {
      const double ref = expected[i].value(points[p][0], points[p][1], points[p][2]);
      if (!equal(values[i][p], ref, 1e-14 * std::max(1.0, std::fabs(ref)))) {
        std::cerr << "for " << expected[i].name << " at point " << p << std::endl;
        correct = false;
      }
    }
  }
  assert(correct);
  return correct ? 0 : 1;
}
