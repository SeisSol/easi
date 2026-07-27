#include "easi/component/ASAGI.h"

#include "easi/util/Print.h"

#include <cmath>
#include <iterator>
#include <sstream>
#include <stdexcept>

namespace easi {

bool ASAGI::accept(int /*unused*/, const Slice<double>& x) const {
  bool acc = true;
  for (unsigned d = 0; d < m_dimensions; ++d) {
    acc = acc && (x(d) >= m_min[d]) && (x(d) <= m_max[d]);
  }
  return acc;
}

void ASAGI::setGrid(const std::set<std::string>& in,
                    const std::vector<std::string>& parameters,
                    asagi::Grid* grid,
                    unsigned numberOfThreads) {
  setIn(in);
  if (dimDomain() != grid->getDimensions()) {
    std::ostringstream os;
    os << "ASAGI requires " << grid->getDimensions() << "D input (got ";
    printWithSeparator(in, os);
    os << ").";
    throw std::invalid_argument(addFileReference(os.str()));
  }
  if (dimDomain() > MaxDimensions) {
    throw std::runtime_error(addFileReference("Unsupported number of dimensions for ASAGI."));
  }

  std::set<std::string> out;
  out.insert(parameters.begin(), parameters.end());
  setOut(out);
  m_numValues = grid->getVarSize() / sizeof(float);
  if (dimCodomain() != m_numValues) {
    std::ostringstream os;
    os << "ASAGI supplies " << m_numValues << "D output (got ";
    printWithSeparator(out, os);
    os << ").";
    throw std::invalid_argument(addFileReference(os.str()));
  }
  if (m_numValues > MaxValues) {
    std::ostringstream os;
    os << "ASAGI supplies " << m_numValues << " values per grid point, but at most " << MaxValues
       << " are supported.";
    throw std::runtime_error(addFileReference(os.str()));
  }

  delete m_grid;
  m_grid = grid;
  m_numberOfThreads = numberOfThreads;
  m_dimensions = grid->getDimensions();

  for (unsigned d = 0; d < m_dimensions; ++d) {
    m_min[d] = grid->getMin(d);
    m_max[d] = grid->getMax(d);

    const double delta = grid->getDelta(d);
    if (std::isfinite(m_min[d]) && std::isfinite(m_max[d]) && std::isfinite(delta) && delta > 0.0) {
      m_origin[d] = m_min[d];
      m_delta[d] = delta;
      m_num[d] = static_cast<unsigned>(std::lround((m_max[d] - m_min[d]) / delta)) + 1;
    } else {
      // ASAGI reports an unbounded dimension; it maps every coordinate to the
      // same grid index, so treat it as a single grid point.
      m_origin[d] = 0.0;
      m_delta[d] = 1.0;
      m_num[d] = 1;
    }
  }

  delete[] m_permutation;
  m_permutation = new unsigned[m_numValues];
  unsigned* perm = m_permutation;
  for (const auto& p : parameters) {
    auto it = out.find(p);
    *perm++ = std::distance(out.begin(), it);
  }
}

void ASAGI::gridGeometry(double* min, double* delta, unsigned* num) const {
  for (unsigned d = 0; d < m_dimensions; ++d) {
    min[d] = m_origin[d];
    delta[d] = m_delta[d];
    num[d] = m_num[d];
  }
}

void ASAGI::sample(const int* index, double* values) const {
  double pos[MaxDimensions] = {};
  // ASAGI stores single precision floats and writes getVarSize() bytes, i.e.
  // m_numValues floats, which setGrid() has bounded by MaxValues.
  float buffer[MaxValues];

  for (unsigned d = 0; d < m_dimensions; ++d) {
    pos[d] = m_origin[d] + index[d] * m_delta[d];
  }
  m_grid->getBuf(buffer, pos);
  for (unsigned v = 0; v < m_numValues; ++v) {
    values[v] = static_cast<double>(buffer[v]);
  }
}

} // namespace easi
