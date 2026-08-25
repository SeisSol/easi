#ifndef EASI_UTIL_INTERPOLATIONKERNEL_H_
#define EASI_UTIL_INTERPOLATIONKERNEL_H_

#include <string>

namespace easi {

/**
 * Interpolation schemes available for grid-based components (ASAGI, SCECFile).
 *
 * All schemes are separable, i.e. the d-dimensional interpolant is the tensor
 * product of d one-dimensional interpolants. A scheme is therefore fully
 * described by its one-dimensional stencil (see InterpolationKernel) and its
 * one-dimensional weight function (see interpolationWeights).
 *
 * Properties on a uniform grid with spacing h:
 *
 *   scheme    stencil   accuracy   smoothness   reproduces
 *   -------   -------   --------   ----------   ----------------------
 *   Nearest      1      O(h)       C^-1         constants
 *   Linear       2      O(h^2)     C^0          polynomials of degree 1
 *   Cubic        4      O(h^3)     C^1          polynomials of degree 2
 *
 * Cubic is Keys' cubic convolution with a = -1/2 (also known as the
 * Catmull-Rom kernel). It is interpolating (it reproduces the sampled values
 * at the grid points exactly) and continuously differentiable, unlike a
 * Lagrange interpolant of the same stencil width, whose derivative jumps at
 * the grid points.
 *
 * Note that Cubic is not bound-preserving: across a sharp contrast it
 * overshoots by roughly 7% of the jump height. Use Linear or Nearest where
 * strict bounds on the interpolated values are required.
 *
 * Reference: R. Keys, "Cubic convolution interpolation for digital image
 * processing", IEEE Trans. Acoust. Speech Signal Process. 29 (1981) 1153-1160.
 */
enum class InterpolationType { Nearest, Linear, Cubic };

/** Largest stencil width over all schemes in InterpolationType. */
constexpr unsigned MaxStencilWidth = 4;

/**
 * Geometry of a one-dimensional interpolation stencil.
 *
 * Let base be the index of the grid point below the query point, i.e.
 * base = floor((x - min) / delta). The stencil then covers the grid indices
 *
 *   base + offset, ..., base + offset + width - 1
 *
 * If roundToNearest is set, base is obtained by rounding instead of flooring
 * and the stencil degenerates to the single point base.
 */
struct InterpolationKernel {
  unsigned width;
  int offset;
  bool roundToNearest;
};

constexpr InterpolationKernel kernelOf(InterpolationType type) {
  switch (type) {
  case InterpolationType::Nearest:
    return {1, 0, true};
  case InterpolationType::Linear:
    return {2, 0, false};
  case InterpolationType::Cubic:
    return {4, -1, false};
  }
  return {2, 0, false};
}

/**
 * Evaluates the one-dimensional weights of the given scheme.
 *
 * @param type   interpolation scheme
 * @param s      local coordinate within the cell, in [0, 1]
 * @param w      output, must hold kernelOf(type).width entries
 *
 * The weights always sum to one, so that constants are reproduced exactly.
 */
inline void interpolationWeights(InterpolationType type, double s, double* w) {
  switch (type) {
  case InterpolationType::Nearest:
    w[0] = 1.0;
    return;
  case InterpolationType::Linear:
    w[0] = 1.0 - s;
    w[1] = s;
    return;
  case InterpolationType::Cubic: {
    // Keys' cubic convolution with a = -1/2, written out for the stencil
    // (base - 1, base, base + 1, base + 2).
    const double s2 = s * s;
    const double s3 = s2 * s;
    w[0] = 0.5 * (-s3 + 2.0 * s2 - s);
    w[1] = 0.5 * (3.0 * s3 - 5.0 * s2 + 2.0);
    w[2] = 0.5 * (-3.0 * s3 + 4.0 * s2 + s);
    w[3] = 0.5 * (s3 - s2);
    return;
  }
  }
}

/**
 * Evaluates linear weights, placed in the stencil slots of a window of the
 * given width.
 *
 * This is the fall-back used wherever the full stencil does not fit into the
 * grid, i.e. in the boundary layer and on grids that are narrower than the
 * stencil. slotOfBase is the slot holding the grid point below the query
 * point; the slot above it receives the remaining weight. All other slots are
 * set to zero, so that the values fetched there do not contribute and no
 * extrapolation takes place.
 */
inline void linearFallbackWeights(unsigned width, unsigned slotOfBase, double s, double* w) {
  for (unsigned j = 0; j < width; ++j) {
    w[j] = 0.0;
  }
  w[slotOfBase] = 1.0 - s;
  if (slotOfBase + 1 < width) {
    w[slotOfBase + 1] = s;
  }
}

/** Parses an interpolation scheme name; throws std::invalid_argument. */
InterpolationType interpolationTypeFromString(const std::string& name);

/** Returns the canonical name of an interpolation scheme. */
const char* interpolationTypeToString(InterpolationType type);

} // namespace easi

#endif
