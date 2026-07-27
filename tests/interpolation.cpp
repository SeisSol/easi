#include "easi/component/Grid.h"
#include "easi/util/InterpolationKernel.h"
#include "easi/util/Matrix.h"

#include <cmath>
#include <cstdio>
#include <functional>
#include <numeric>
#include <set>
#include <string>
#include <vector>

namespace {

int failures = 0;

void check(bool ok, const std::string& what) {
  if (!ok) {
    std::fprintf(stderr, "FAILED: %s\n", what.c_str());
    ++failures;
  }
}

void checkClose(double actual, double expected, double tol, const std::string& what) {
  if (!(std::fabs(actual - expected) <= tol)) {
    std::fprintf(stderr,
                 "FAILED: %s (got %.17g, expected %.17g, tolerance %g)\n",
                 what.c_str(),
                 actual,
                 expected,
                 tol);
    ++failures;
  }
}

using SampleFunction = std::function<double(const double*, unsigned)>;

/**
 * A grid backend that tabulates an analytic function.
 *
 * This is also the reference for what Grid<Derived> expects of a backend:
 * gridGeometry(), sample() and permutation().
 */
class TestGrid : public easi::Grid<TestGrid> {
  public:
  TestGrid(const std::vector<double>& min,
           const std::vector<double>& delta,
           const std::vector<unsigned>& num,
           unsigned numValues,
           const SampleFunction& f)
      : m_min(min), m_delta(delta), m_num(num), m_numValues(numValues) {
    static const char* const coordinateNames[] = {"a", "b", "c", "d", "e", "f"};
    static const char* const valueNames[] = {"p", "q", "r", "s", "t", "u"};

    std::set<std::string> in;
    std::set<std::string> out;
    for (unsigned d = 0; d < m_num.size(); ++d) {
      in.insert(coordinateNames[d]);
    }
    for (unsigned v = 0; v < numValues; ++v) {
      out.insert(valueNames[v]);
    }
    setIn(in);
    setOut(out);

    m_stride.resize(m_num.size());
    std::size_t total = 1;
    for (unsigned d = 0; d < m_num.size(); ++d) {
      m_stride[d] = total;
      total *= m_num[d];
    }

    m_values.resize(total * numValues);
    std::vector<double> position(m_num.size());
    for (std::size_t flat = 0; flat < total; ++flat) {
      std::size_t rest = flat;
      for (unsigned d = 0; d < m_num.size(); ++d) {
        position[d] = m_min[d] + static_cast<double>(rest % m_num[d]) * m_delta[d];
        rest /= m_num[d];
      }
      for (unsigned v = 0; v < numValues; ++v) {
        m_values[flat * numValues + v] = f(position.data(), v);
      }
    }
  }

  void gridGeometry(double* min, double* delta, unsigned* num) const {
    for (unsigned d = 0; d < m_num.size(); ++d) {
      min[d] = m_min[d];
      delta[d] = m_delta[d];
      num[d] = m_num[d];
    }
  }

  void sample(const int* index, double* values) const {
    std::size_t flat = 0;
    for (unsigned d = 0; d < m_num.size(); ++d) {
      // The base class must never hand out an out-of-range index.
      check(index[d] >= 0 && index[d] < static_cast<int>(m_num[d]),
            "sample() received an out-of-range index");
      flat += static_cast<std::size_t>(index[d]) * m_stride[d];
    }
    for (unsigned v = 0; v < m_numValues; ++v) {
      values[v] = m_values[flat * m_numValues + v];
    }
  }

  unsigned permutation(unsigned index) const { return index; }

  /** Evaluates the component at the given points; returns value v of point i. */
  std::vector<double> interpolate(const std::vector<std::vector<double>>& points) {
    const unsigned dim = static_cast<unsigned>(m_num.size());
    easi::Matrix<double> x(static_cast<unsigned>(points.size()), dim);
    for (unsigned i = 0; i < points.size(); ++i) {
      for (unsigned d = 0; d < dim; ++d) {
        x(i, d) = points[i][d];
      }
    }
    easi::Matrix<double> y = map(x);

    std::vector<double> result(points.size() * m_numValues);
    for (unsigned i = 0; i < points.size(); ++i) {
      for (unsigned v = 0; v < m_numValues; ++v) {
        result[i * m_numValues + v] = y(i, v);
      }
    }
    return result;
  }

  protected:
  unsigned numberOfThreads() const override { return 1; }

  private:
  std::vector<double> m_min;
  std::vector<double> m_delta;
  std::vector<unsigned> m_num;
  std::vector<std::size_t> m_stride;
  unsigned m_numValues;
  std::vector<double> m_values;
};

// --------------------------------------------------------------------------

void testCubicWeights() {
  // At the centre of a cell the Keys kernel must reduce to the classical
  // (-1/16, 9/16, 9/16, -1/16) stencil.
  const std::vector<double> nodeValues = {3.0, -1.0, 4.0, 1.0, -5.0, 9.0};
  auto f = [&](const double* p, unsigned) {
    return nodeValues[static_cast<std::size_t>(std::lround(p[0]))];
  };
  TestGrid grid({0.0}, {1.0}, {6}, 1, f);
  grid.setInterpolationType(easi::InterpolationType::Cubic);

  const auto y = grid.interpolate({{2.5}});
  const double expected = -1.0 / 16.0 * nodeValues[1] + 9.0 / 16.0 * nodeValues[2] +
                          9.0 / 16.0 * nodeValues[3] - 1.0 / 16.0 * nodeValues[4];
  checkClose(y[0], expected, 1e-14, "cubic weights at the cell centre");

  // The weights must form a partition of unity for every local coordinate.
  for (int i = 0; i <= 20; ++i) {
    const double s = i / 20.0;
    double w[easi::MaxStencilWidth];
    easi::interpolationWeights(easi::InterpolationType::Cubic, s, w);
    const double sum = std::accumulate(w, w + 4, 0.0);
    checkClose(sum, 1.0, 1e-14, "cubic weights sum to one");
  }
}

void testCardinality() {
  auto f = [](const double* p, unsigned) { return std::sin(1.7 * p[0]) + 0.3 * p[0]; };
  const easi::InterpolationType types[] = {easi::InterpolationType::Nearest,
                                           easi::InterpolationType::Linear,
                                           easi::InterpolationType::Cubic};
  for (auto type : types) {
    TestGrid grid({0.0}, {0.25}, {12}, 1, f);
    grid.setInterpolationType(type);
    for (unsigned i = 0; i < 12; ++i) {
      const double x = 0.25 * i;
      const auto y = grid.interpolate({{x}});
      checkClose(y[0],
                 f(&x, 0),
                 1e-13,
                 std::string("interpolant reproduces node values (") +
                     easi::interpolationTypeToString(type) + ")");
    }
  }
}

void testPolynomialReproduction() {
  // Cubic (Keys, a = -1/2) is third order accurate and therefore reproduces
  // polynomials up to degree two exactly; linear only up to degree one.
  struct Case {
    easi::InterpolationType type;
    unsigned maxDegree;
  };
  const Case cases[] = {{easi::InterpolationType::Linear, 1}, {easi::InterpolationType::Cubic, 2}};

  for (const auto& c : cases) {
    for (unsigned degree = 0; degree <= 3; ++degree) {
      auto f = [degree](const double* p, unsigned) { return std::pow(p[0], degree); };
      TestGrid grid({-2.0}, {0.5}, {16}, 1, f);
      grid.setInterpolationType(c.type);

      double maxError = 0.0;
      for (int i = 0; i <= 40; ++i) {
        // Stay in the interior so that the full stencil is used.
        const double x = -1.0 + 4.0 * i / 40.0;
        const auto y = grid.interpolate({{x}});
        maxError = std::max(maxError, std::fabs(y[0] - std::pow(x, degree)));
      }

      const std::string what = std::string(easi::interpolationTypeToString(c.type)) +
                               " reproduction of x^" + std::to_string(degree);
      if (degree <= c.maxDegree) {
        checkClose(maxError, 0.0, 1e-12, what + " (must be exact)");
      } else {
        check(maxError > 1e-6, what + " (must not be exact)");
      }
    }
  }
}

void testConvergenceOrder() {
  auto f = [](const double* p, unsigned) { return std::sin(3.1 * p[0]) * std::exp(0.7 * p[1]); };
  struct Case {
    easi::InterpolationType type;
    double expectedOrder;
  };
  const Case cases[] = {{easi::InterpolationType::Linear, 2.0},
                        {easi::InterpolationType::Cubic, 3.0}};

  for (const auto& c : cases) {
    std::vector<double> errors;
    for (unsigned n : {17u, 33u, 65u, 129u}) {
      const double h = 1.0 / (n - 1);
      TestGrid grid({0.0, 0.0}, {h, h}, {n, n}, 1, f);
      grid.setInterpolationType(c.type);

      std::vector<std::vector<double>> points;
      for (int i = 0; i < 37; ++i) {
        for (int j = 0; j < 37; ++j) {
          // Interior only; the boundary layer falls back to linear by design.
          points.push_back({0.2 + 0.6 * i / 36.0, 0.2 + 0.6 * j / 36.0});
        }
      }
      const auto y = grid.interpolate(points);

      double maxError = 0.0;
      for (std::size_t k = 0; k < points.size(); ++k) {
        maxError = std::max(maxError, std::fabs(y[k] - f(points[k].data(), 0)));
      }
      errors.push_back(maxError);
    }

    const double order = std::log2(errors[errors.size() - 2] / errors.back());
    std::printf("  %-8s order %.2f (errors %.3e -> %.3e)\n",
                easi::interpolationTypeToString(c.type),
                order,
                errors.front(),
                errors.back());
    check(std::fabs(order - c.expectedOrder) < 0.25,
          std::string("convergence order of ") + easi::interpolationTypeToString(c.type));
  }
}

void testBoundaryFallback() {
  auto f = [](const double* p, unsigned) { return std::sin(1.3 * p[0]) + 0.5 * p[0] * p[0]; };
  TestGrid cubic({0.0}, {0.5}, {10}, 1, f);
  cubic.setInterpolationType(easi::InterpolationType::Cubic);
  TestGrid linear({0.0}, {0.5}, {10}, 1, f);
  linear.setInterpolationType(easi::InterpolationType::Linear);

  // First and last cell: the cubic stencil does not fit, so the result must
  // agree with linear interpolation exactly.
  for (double x : {0.05, 0.25, 0.49, 4.05, 4.3, 4.49}) {
    const auto yc = cubic.interpolate({{x}});
    const auto yl = linear.interpolate({{x}});
    checkClose(yc[0], yl[0], 1e-15, "boundary layer falls back to linear");
  }

  // Second cell inwards: the stencil fits, so the results must differ.
  const auto yc = cubic.interpolate({{0.75}});
  const auto yl = linear.interpolate({{0.75}});
  check(std::fabs(yc[0] - yl[0]) > 1e-9, "interior does not fall back to linear");
}

void testOutOfDomainClamping() {
  auto f = [](const double* p, unsigned) { return 2.0 * p[0] + 1.0; };
  TestGrid grid({0.0}, {1.0}, {5}, 1, f);
  grid.setInterpolationType(easi::InterpolationType::Cubic);

  const auto below = grid.interpolate({{-10.0}});
  checkClose(below[0], 1.0, 1e-13, "clamping below the grid");
  const auto above = grid.interpolate({{10.0}});
  checkClose(above[0], 9.0, 1e-13, "clamping above the grid");
}

void testNearest() {
  auto f = [](const double* p, unsigned) { return std::round(p[0]); };
  TestGrid grid({0.0}, {1.0}, {6}, 1, f);
  grid.setInterpolationType(easi::InterpolationType::Nearest);

  checkClose(grid.interpolate({{2.4}})[0], 2.0, 1e-15, "nearest rounds down");
  checkClose(grid.interpolate({{2.6}})[0], 3.0, 1e-15, "nearest rounds up");
}

void testThreeDimensionalAndMultipleValues() {
  // A triquadratic polynomial must be reproduced exactly by the cubic scheme,
  // which exercises the tensor product reduction in all three dimensions.
  auto f = [](const double* p, unsigned v) {
    const double base =
        (1.0 + p[0] + p[0] * p[0]) * (2.0 - p[1] + p[1] * p[1]) * (0.5 + 2.0 * p[2] + p[2] * p[2]);
    return (v == 0) ? base : -3.0 * base + 7.0;
  };
  TestGrid grid({-1.0, -1.0, -1.0}, {0.4, 0.5, 0.3}, {12, 11, 13}, 2, f);
  grid.setInterpolationType(easi::InterpolationType::Cubic);

  std::vector<std::vector<double>> points;
  for (int i = 0; i < 6; ++i) {
    for (int j = 0; j < 6; ++j) {
      for (int k = 0; k < 6; ++k) {
        points.push_back({-0.3 + 0.1 * i, -0.2 + 0.13 * j, -0.4 + 0.11 * k});
      }
    }
  }
  const auto y = grid.interpolate(points);
  double maxError = 0.0;
  for (std::size_t n = 0; n < points.size(); ++n) {
    for (unsigned v = 0; v < 2; ++v) {
      maxError = std::max(maxError, std::fabs(y[n * 2 + v] - f(points[n].data(), v)));
    }
  }
  checkClose(maxError, 0.0, 1e-11, "3D triquadratic reproduction with two values");
}

void testStencilCache() {
  // The stencil cache keys on the base cell, so a scrambled point order must
  // give bitwise the same answer as a clustered one.
  auto f = [](const double* p, unsigned) { return std::sin(2.0 * p[0]) * std::cos(1.4 * p[1]); };
  TestGrid grid({0.0, 0.0}, {0.25, 0.25}, {20, 20}, 1, f);
  grid.setInterpolationType(easi::InterpolationType::Cubic);

  std::vector<std::vector<double>> clustered;
  for (int i = 0; i < 64; ++i) {
    clustered.push_back({1.05 + 0.001 * (i % 8), 2.05 + 0.001 * (i / 8)});
  }
  std::vector<std::vector<double>> scrambled;
  for (int i = 0; i < 64; ++i) {
    scrambled.push_back(clustered[(i * 37) % 64]);
  }

  const auto a = grid.interpolate(clustered);
  const auto b = grid.interpolate(scrambled);
  for (int i = 0; i < 64; ++i) {
    check(a[(i * 37) % 64] == b[i], "stencil cache is order independent");
  }

  // Evaluating one point at a time must give the same result as in a batch.
  for (int i = 0; i < 64; ++i) {
    const auto single = grid.interpolate({clustered[i]});
    check(single[0] == a[i], "stencil cache does not depend on batching");
  }
}

void testDegenerateDimension() {
  // A grid with a single point along one axis must still work.
  auto f = [](const double* p, unsigned) { return 3.0 + p[0]; };
  TestGrid grid({0.0, 5.0}, {1.0, 1.0}, {6, 1}, 1, f);
  grid.setInterpolationType(easi::InterpolationType::Cubic);
  checkClose(grid.interpolate({{2.0, 5.0}})[0], 5.0, 1e-13, "degenerate dimension, on the grid");
  checkClose(grid.interpolate({{2.0, 42.0}})[0], 5.0, 1e-13, "degenerate dimension, off the grid");
}

void testTypeParsing() {
  check(easi::interpolationTypeFromString("CUBIC") == easi::InterpolationType::Cubic,
        "interpolation type parsing is case insensitive");
  check(easi::interpolationTypeFromString("nearest") == easi::InterpolationType::Nearest,
        "interpolation type parsing accepts nearest");
  bool threw = false;
  try {
    easi::interpolationTypeFromString("quintic");
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  check(threw, "unknown interpolation type is rejected");
}

} // namespace

int main() {
  testCubicWeights();
  testCardinality();
  testPolynomialReproduction();
  testConvergenceOrder();
  testBoundaryFallback();
  testOutOfDomainClamping();
  testNearest();
  testThreeDimensionalAndMultipleValues();
  testStencilCache();
  testDegenerateDimension();
  testTypeParsing();

  if (failures != 0) {
    std::fprintf(stderr, "%d check(s) failed.\n", failures);
    return 1;
  }
  std::printf("All interpolation checks passed.\n");
  return 0;
}
