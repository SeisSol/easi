#include "easi/component/ASAGI.h"
#include "easi/util/Print.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <sstream>
#include <stdexcept>

namespace easi {

bool ASAGI::accept(int, Slice<double> const& x) const {
    bool acc = true;
    for (unsigned d = 0; d < m_grid->getDimensions(); ++d) {
        acc = acc && (x(d) >= m_min[d]) && (x(d) <= m_max[d]);
    }
    return acc;
}

void ASAGI::setGrid(std::set<std::string> const& in, std::vector<std::string> const& parameters,
                    asagi::Grid* grid, unsigned numberOfThreads) {
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
    for (auto const& p : parameters) {
        auto it = std::find(out.begin(), out.end(), p);
        *perm++ = std::distance(out.begin(), it);
    }
}

void ASAGI::getNearestNeighbour(Slice<double> const& x, double* buffer) {
    double pos[MaxDimensions];
    float* bufferSP = reinterpret_cast<float*>(buffer);
    for (unsigned d = 0; d < m_grid->getDimensions(); ++d) {
        pos[d] = x(d);
    }
    m_grid->getBuf(bufferSP, pos);
    for (int j = m_numValues - 1; j >= 0; --j) {
        buffer[j] = static_cast<double>(bufferSP[j]);
    }
}

void ASAGI::getNeighbours(Slice<double> const& x, double* weights, double* buffer) {
    double lowPos[MaxDimensions];
    for (unsigned d = 0; d < m_grid->getDimensions(); ++d) {
        lowPos[d] = m_min[d] + std::floor((x(d) - m_min[d]) * m_deltaInv[d]) * m_delta[d];
        weights[d] = (x(d) - lowPos[d]) * m_deltaInv[d];
    }

    double pos[MaxDimensions];
    for (unsigned i = 0; i < (1u << m_grid->getDimensions()); ++i) {
        float* bufferSP = reinterpret_cast<float*>(buffer + i * m_numValues);
        for (unsigned d = 0; d < m_grid->getDimensions(); ++d) {
            pos[d] = std::min(lowPos[d] + ((i & (1 << d)) >> d) * m_delta[d], m_max[d]);
        }
        m_grid->getBuf(bufferSP, pos);
        for (int j = m_numValues - 1; j >= 0; --j) {
            buffer[i * m_numValues + j] = static_cast<double>(bufferSP[j]);
        }
    }
}

} // namespace easi
