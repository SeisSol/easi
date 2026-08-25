#include "easi/util/RegularGrid.h"

#include <sstream>
#include <stdexcept>

namespace easi {

void RegularGrid::allocate(const unsigned* numGridPoints, unsigned dimensions, unsigned numValues) {
  if (dimensions > MaxDimensions) {
    throw std::runtime_error("Unsupported number of dimensions for RegularGrid.");
  }
  m_dimensions = dimensions;
  m_numValues = numValues;
  unsigned size = m_numValues;
  for (unsigned d = 0; d < m_dimensions; ++d) {
    if (numGridPoints[d] == 0) {
      std::ostringstream os;
      os << "RegularGrid has no grid points in dimension " << d << ".";
      throw std::runtime_error(os.str());
    }
    m_num[d] = numGridPoints[d];
    size *= m_num[d];
  }
  delete[] m_values;
  m_values = new double[size];
}

void RegularGrid::setVolume(const double* min, const double* max) {
  for (unsigned d = 0; d < m_dimensions; ++d) {
    m_min[d] = min[d];
    m_max[d] = max[d];
    // A single grid point carries no spacing information; use a positive
    // placeholder so that the interpolation, which then always sees s = 0,
    // stays well defined.
    m_delta[d] = (m_num[d] > 1) ? (max[d] - min[d]) / (m_num[d] - 1) : 1.0;
  }
}

double* RegularGrid::operator()(const unsigned* index) {
  unsigned stride = 1;
  unsigned idx = 0;
  for (unsigned d = 0; d < m_dimensions; ++d) {
    idx += index[d] * stride;
    stride *= m_num[d];
  }
  return m_values + m_numValues * idx;
}

void RegularGrid::gridGeometry(double* min, double* delta, unsigned* num) const {
  for (unsigned d = 0; d < m_dimensions; ++d) {
    min[d] = m_min[d];
    delta[d] = m_delta[d];
    num[d] = m_num[d];
  }
}

void RegularGrid::sample(const int* index, double* values) const {
  unsigned stride = 1;
  unsigned idx = 0;
  for (unsigned d = 0; d < m_dimensions; ++d) {
    idx += static_cast<unsigned>(index[d]) * stride;
    stride *= m_num[d];
  }
  const double* entry = m_values + m_numValues * idx;
  for (unsigned v = 0; v < m_numValues; ++v) {
    values[v] = entry[v];
  }
}

} // namespace easi
