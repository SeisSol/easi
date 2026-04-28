#include "easi/component/ASAGI.h"

#include "easi/util/Print.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <sstream>
#include <stdexcept>

namespace easi {

bool ASAGI::accept(int /*unused*/, const Slice<double>& x) const {
  bool acc = true;
  for (unsigned d = 0; d < m_grid->getDimensions(); ++d) {
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

  delete m_grid;
  m_grid = grid;
  m_numberOfThreads = numberOfThreads;

  for (unsigned d = 0; d < grid->getDimensions(); ++d) {
    m_min[d] = grid->getMin(d);
    m_max[d] = grid->getMax(d);
    m_delta[d] = grid->getDelta(d);
    m_deltaInv[d] = 1.0 / m_delta[d];
  }

  delete[] m_permutation;
  m_permutation = new unsigned[m_numValues];
  unsigned* perm = m_permutation;
  for (const auto& p : parameters) {
    auto it = out.find(p);
    *perm++ = std::distance(out.begin(), it);
  }
}

void ASAGI::getNearestNeighbor(const Slice<double>& x, double* buffer) {
  double pos[MaxDimensions]{};
  float bufferSP[MaxDimensions]{};
  for (unsigned d = 0; d < m_grid->getDimensions(); ++d) {
    pos[d] = x(d);
  }
  m_grid->getBuf(bufferSP, pos);
  for (int j = 0; j < m_numValues; ++j) {
    buffer[j] = static_cast<double>(bufferSP[j]);
  }
}

void ASAGI::getNeighbors(const Slice<double>& x, double* weights, double* buffer) {
  double lowPos[MaxDimensions]{};
  for (unsigned d = 0; d < m_grid->getDimensions(); ++d) {
    lowPos[d] = m_min[d] + std::floor((x(d) - m_min[d]) * m_deltaInv[d]) * m_delta[d];
    weights[d] = (x(d) - lowPos[d]) * m_deltaInv[d];
  }

  double pos[MaxDimensions]{};
  float bufferSP[MaxDimensions]{};
  for (unsigned i = 0; i < (1U << m_grid->getDimensions()); ++i) {
    for (unsigned d = 0; d < m_grid->getDimensions(); ++d) {
      pos[d] = std::min(lowPos[d] + ((i & (1 << d)) >> d) * m_delta[d], m_max[d]);
    }
    m_grid->getBuf(bufferSP, pos);
    for (int j = 0; j < m_numValues; ++j) {
      buffer[i * m_numValues + j] = static_cast<double>(bufferSP[j]);
    }
  }
}

} // namespace easi
