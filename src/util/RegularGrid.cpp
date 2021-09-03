#include "easi/util/RegularGrid.h"
#include "easi/util/Slice.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <stdexcept>

namespace easi {

void RegularGrid::allocate(unsigned const* numGridPoints, unsigned dimensions, unsigned numValues) {
    if (dimensions > MaxDimensions) {
        throw std::runtime_error("Unsupported number of dimensions for RegularGrid.");
    }
    m_dimensions = dimensions;
    m_numValues = numValues;
    unsigned size = m_numValues;
    for (unsigned d = 0; d < m_dimensions; ++d) {
        m_num[d] = numGridPoints[d];
        size *= m_num[d];
    }
    m_values = new double[size];
}

void RegularGrid::setVolume(double const* min, double const* max) {
    for (unsigned d = 0; d < m_dimensions; ++d) {
        m_min[d] = min[d];
        m_max[d] = max[d];
        m_delta[d] = (max[d] - min[d]) / (m_num[d] - 1);
    }
}

double* RegularGrid::operator()(unsigned const* index) {
    unsigned stride = 1;
    unsigned idx = 0;
    for (unsigned d = 0; d < m_dimensions; ++d) {
        idx += index[d] * stride;
        stride *= m_num[d];
    }
    return m_values + m_numValues * idx;
}

void RegularGrid::getNearestNeighbour(Slice<double> const& x, double* buffer) {
    assert(x.size() == m_dimensions);

    unsigned idx[MaxDimensions];
    for (unsigned d = 0; d < m_dimensions; ++d) {
        if (x(d) < m_min[d]) {
            idx[d] = 0;
        } else if (x(d) >= m_max[d]) {
            idx[d] = m_num[d] - 1;
        } else {
            double xn = (x(d) - m_min[d]) / m_delta[d];
            idx[d] = std::round(xn);
        }
    }

    double* values = operator()(idx);
    for (unsigned v = 0; v < m_numValues; ++v) {
        buffer[v] = values[v];
    }
}

void RegularGrid::getNeighbours(Slice<double> const& x, double* weights, double* buffer) {
    assert(x.size() == m_dimensions);

    unsigned idxBase[MaxDimensions];
    for (unsigned d = 0; d < m_dimensions; ++d) {
        if (x(d) < m_min[d]) {
            idxBase[d] = 0;
            weights[d] = 0.0;
        } else if (x(d) >= m_max[d]) {
            idxBase[d] = m_num[d] - 1;
            weights[d] = 1.0;
        } else {
            double xn = (x(d) - m_min[d]) / m_delta[d];
            idxBase[d] = xn;
            weights[d] = xn - idxBase[d];
        }
    }

    unsigned idx[MaxDimensions];
    for (unsigned i = 0; i < (1u << m_dimensions); ++i) {
        for (unsigned d = 0; d < m_dimensions; ++d) {
            idx[d] = std::min(idxBase[d] + ((i & (1 << d)) >> d), m_num[d] - 1);
        }
        double* values = operator()(idx);
        for (unsigned v = 0; v < m_numValues; ++v) {
            buffer[i * m_numValues + v] = values[v];
        }
    }
}

} // namespace easi
