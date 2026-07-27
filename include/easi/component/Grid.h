#ifndef EASI_COMPONENT_GRID_H_
#define EASI_COMPONENT_GRID_H_

#include "easi/component/Map.h"
#include "easi/util/InterpolationKernel.h"
#include "easi/util/Matrix.h"
#include "easi/util/Slice.h"

#include <cmath>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace easi {

/**
 * Base class for components that interpolate on a uniform Cartesian grid.
 *
 * Derived classes (the grid backends) must provide
 *
 *   void gridGeometry(double* min, double* delta, unsigned* num) const;
 *   void sample(const int* index, double* values) const;
 *   unsigned permutation(unsigned index) const;
 *
 * where
 *
 *   gridGeometry  fills dimDomain() entries describing the grid: the
 *                 coordinate of grid point 0, the (positive) spacing, and the
 *                 number of grid points along each axis;
 *   sample        writes the dimCodomain() values stored at the given grid
 *                 index into values. The index is guaranteed to lie within
 *                 [0, num[d] - 1] in every dimension, so backends need not
 *                 range-check it;
 *   permutation   maps the position of a value within sample()'s output to
 *                 its column in the result matrix.
 *
 * All index arithmetic, weighting and boundary handling lives here, so that a
 * backend only has to know how to read a single grid point. This class is
 * responsible for never asking for a point outside the grid.
 */
template <typename Derived>
class Grid : public Map {
  public:
  static constexpr unsigned MaxDimensions = 6;

  virtual ~Grid() {}

  virtual Matrix<double> map(Matrix<double>& x);

  void setInterpolationType(const std::string& interpolationType) {
    m_interpolationType = interpolationTypeFromString(interpolationType);
  }
  void setInterpolationType(InterpolationType interpolationType) {
    m_interpolationType = interpolationType;
  }
  InterpolationType interpolationType() const { return m_interpolationType; }

  protected:
  virtual unsigned numberOfThreads() const = 0;

  private:
  /**
   * Interpolation for one fixed scheme. The scheme is a template parameter so
   * that the stencil width, and with it the innermost loop, is known at
   * compile time.
   */
  template <InterpolationType Type>
  Matrix<double> mapWith(Matrix<double>& x);

  void gridGeometry(double* min, double* delta, unsigned* num) const {
    static_cast<const Derived*>(this)->gridGeometry(min, delta, num);
  }
  void sample(const int* index, double* values) const {
    static_cast<const Derived*>(this)->sample(index, values);
  }
  unsigned permutation(unsigned index) const {
    return static_cast<const Derived*>(this)->permutation(index);
  }

  InterpolationType m_interpolationType = InterpolationType::Linear;
};

template <typename GridImpl>
Matrix<double> Grid<GridImpl>::map(Matrix<double>& x) {
  switch (m_interpolationType) {
  case InterpolationType::Nearest:
    return mapWith<InterpolationType::Nearest>(x);
  case InterpolationType::Cubic:
    return mapWith<InterpolationType::Cubic>(x);
  case InterpolationType::Linear:
    break;
  }
  return mapWith<InterpolationType::Linear>(x);
}

template <typename GridImpl>
template <InterpolationType Type>
Matrix<double> Grid<GridImpl>::mapWith(Matrix<double>& x) {
  constexpr InterpolationKernel Kernel = kernelOf(Type);
  constexpr unsigned Width = Kernel.width;

  const unsigned dim = dimDomain();
  const unsigned numValues = dimCodomain();

  Matrix<double> y(x.rows(), numValues);

  if (dim > MaxDimensions) {
    throw std::runtime_error(addFileReference("Unsupported number of dimensions for Grid."));
  }

  double min[MaxDimensions];
  double delta[MaxDimensions];
  double deltaInv[MaxDimensions];
  unsigned num[MaxDimensions];
  gridGeometry(min, delta, num);

  for (unsigned d = 0; d < dim; ++d) {
    if (!(delta[d] > 0.0) || num[d] == 0) {
      std::ostringstream os;
      os << "Degenerate grid in dimension " << d << " (spacing " << delta[d] << ", " << num[d]
         << " points).";
      throw std::runtime_error(addFileReference(os.str()));
    }
    deltaInv[d] = 1.0 / delta[d];
  }

  unsigned stencilSize = 1;
  for (unsigned d = 0; d < dim; ++d) {
    stencilSize *= Width;
  }

#ifdef _OPENMP
#pragma omp parallel num_threads(numberOfThreads()) shared(x, y)
#endif
  {
    // Values of the current stencil, flattened as
    //   flat = sum_d stencilIndex[d] * Width^d,   dimension 0 varying fastest.
    std::vector<double> stencil(stencilSize * numValues);
    // Scratch for the tensor product reduction. The first pass reads from
    // stencil and writes here, which leaves stencil available for reuse.
    std::vector<double> work(stencilSize * numValues);

    double weights[MaxDimensions * MaxStencilWidth];
    // First grid index of the stencil window along each axis. It determines
    // the gathered values completely and is therefore the cache key.
    int start[MaxDimensions];

    // Single-entry stencil cache. Query points are usually spatially
    // clustered (e.g. the quadrature points of one element), and the grid is
    // usually coarser than the mesh, so consecutive points tend to share a
    // cell. This matters most for backends whose sample() is expensive, such
    // as ASAGI.
    int cachedStart[MaxDimensions];
    bool cacheValid = false;

#ifdef _OPENMP
#pragma omp for
#endif
    for (unsigned i = 0; i < x.rows(); ++i) {
      const Slice<double> xi = x.rowSlice(i);

      // --- locate the query point and evaluate the 1D weights ---------------
      for (unsigned d = 0; d < dim; ++d) {
        double* w = weights + d * MaxStencilWidth;

        // Position in grid coordinates, clamped into [0, num - 1]. Points
        // outside the grid are pulled onto the boundary, i.e. the interpolant
        // is extended constantly. The clamp is written so that a non-finite
        // coordinate ends up at 0 rather than overflowing the cast below.
        const double top = static_cast<double>(num[d] - 1);
        double raw = (xi(d) - min[d]) * deltaInv[d];
        if (!(raw > 0.0)) {
          raw = 0.0;
        } else if (raw > top) {
          raw = top;
        }

        if (Kernel.roundToNearest) {
          start[d] = static_cast<int>(std::lround(raw));
          w[0] = 1.0;
          continue;
        }

        const long maxBase = (num[d] >= 2) ? static_cast<long>(num[d]) - 2 : 0;
        const double lower = std::floor(raw);
        long index = static_cast<long>(lower);
        double s;
        if (index > maxBase) {
          index = maxBase;
          s = (num[d] >= 2) ? 1.0 : 0.0;
        } else {
          s = raw - lower;
        }

        // Shift the stencil window so that it lies inside the grid. Where the
        // window had to be shifted, or where the grid is narrower than the
        // stencil, fall back to linear interpolation along this axis only.
        // This keeps the full order along the remaining axes and never
        // extrapolates.
        long windowStart;
        if constexpr (Width <= 2) {
          // The window is the cell itself, so it never needs shifting: index
          // is already bounded by num - 2. On an axis with a single grid
          // point, s is zero and the weight of the upper node vanishes.
          windowStart = index;
          interpolationWeights(Type, s, w);
        } else {
          const long maxStart = static_cast<long>(num[d]) - static_cast<long>(Width);
          windowStart = (num[d] >= Width) ? (index + Kernel.offset) : 0;
          if (windowStart < 0) {
            windowStart = 0;
          } else if (windowStart > maxStart) {
            windowStart = maxStart > 0 ? maxStart : 0;
          }
          const unsigned slotOfBase = static_cast<unsigned>(index - windowStart);
          if (num[d] >= Width && slotOfBase == static_cast<unsigned>(-Kernel.offset)) {
            interpolationWeights(Type, s, w);
          } else {
            linearFallbackWeights(Width, slotOfBase, s, w);
          }
        }
        start[d] = static_cast<int>(windowStart);
      }

      // --- gather the stencil ----------------------------------------------
      bool reuse = cacheValid;
      for (unsigned d = 0; d < dim && reuse; ++d) {
        reuse = (start[d] == cachedStart[d]);
      }

      if (!reuse) {
        // Grid index of every stencil slot along every axis. Clamping happens
        // here, once per axis, rather than inside the gather loop. Only grids
        // narrower than the stencil can need it at all.
        int nodeIndex[MaxDimensions * MaxStencilWidth];
        for (unsigned d = 0; d < dim; ++d) {
          for (unsigned j = 0; j < Width; ++j) {
            const long g = static_cast<long>(start[d]) + static_cast<long>(j);
            nodeIndex[d * Width + j] =
                static_cast<int>(g < static_cast<long>(num[d]) ? g : static_cast<long>(num[d]) - 1);
          }
        }

        int stencilIndex[MaxDimensions] = {};
        int gridIndex[MaxDimensions];
        for (unsigned f = 0; f < stencilSize; ++f) {
          for (unsigned d = 0; d < dim; ++d) {
            gridIndex[d] = nodeIndex[d * Width + static_cast<unsigned>(stencilIndex[d])];
          }
          sample(gridIndex, stencil.data() + f * numValues);

          for (unsigned d = 0; d < dim; ++d) {
            if (++stencilIndex[d] < static_cast<int>(Width)) {
              break;
            }
            stencilIndex[d] = 0;
          }
        }
        for (unsigned d = 0; d < dim; ++d) {
          cachedStart[d] = start[d];
        }
        cacheValid = true;
      }

      // --- tensor product reduction, one dimension at a time ----------------
      // The first pass consumes the cached stencil without modifying it; all
      // later passes reduce work in place, which is safe because the target
      // index p is always smaller than every source index p + k * count.
      const double* source = stencil.data();
      unsigned count = stencilSize;
      for (int d = static_cast<int>(dim) - 1; d >= 0; --d) {
        count /= Width;
        const double* w = weights + static_cast<unsigned>(d) * MaxStencilWidth;
        for (unsigned p = 0; p < count; ++p) {
          for (unsigned v = 0; v < numValues; ++v) {
            double acc = 0.0;
            for (unsigned k = 0; k < Width; ++k) {
              acc += w[k] * source[(p + k * count) * numValues + v];
            }
            work[p * numValues + v] = acc;
          }
        }
        source = work.data();
      }

      for (unsigned v = 0; v < numValues; ++v) {
        y(i, permutation(v)) = work[v];
      }
    }
  }

  return y;
}

} // namespace easi

#endif
