#include "easi/component/AffineMap.h"

#include <limits>
#include <stdexcept>
#include <utility>

namespace easi {

Matrix<double> AffineMap::map(Matrix<double>& x) {
    Matrix<double> y = x * m_matrix;
    for (unsigned i = 0; i < y.rows(); ++i) {
        for (unsigned j = 0; j < y.cols(); ++j) {
            y(i, j) += m_translation(j);
        }
    }
    return y;
}

void AffineMap::setMap(std::set<std::string> const& in, Transformation const& matrix,
                       Translation const& translation) {
    if (matrix.size() != translation.size()) {
        throw std::invalid_argument(
            addFileReference("Matrix and translation must have the same size in an affine map."));
    }

    setIn(in);

    std::set<std::string> out;
    unsigned nCoeffs = std::numeric_limits<unsigned>::max();
    for (auto const& kv : matrix) {
        out.insert(kv.first);
        if (nCoeffs != std::numeric_limits<unsigned>::max() && kv.second.size() != nCoeffs) {
            throw std::invalid_argument(
                addFileReference("The matrix in a affine map must have the same number of "
                                 "coefficients for each entry."));
        }
        nCoeffs = kv.second.size();
    }
    setOut(out);

    if (nCoeffs != dimDomain()) {
        throw std::invalid_argument(addFileReference(
            "Number of matrix entries does not match number of input parameters."));
    }

    m_matrix.reallocate(dimDomain(), dimCodomain());
    m_translation.reallocate(dimCodomain());

    unsigned col = 0;
    for (auto const& kv : matrix) {
        unsigned row = 0;
        for (auto const& v : kv.second) {
            m_matrix(row, col) = v;
            ++row;
        }
        ++col;
    }

    unsigned row = 0;
    for (auto const& kv : translation) {
        m_translation(row++) = kv.second;
    }
}

} // namespace easi
