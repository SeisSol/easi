#include "easi/component/PolynomialMap.h"
#include "easi/util/Print.h"

#include <limits>
#include <ostream>
#include <stdexcept>
#include <utility>

namespace easi {

// Implements Horner's method
Matrix<double> PolynomialMap::map(Matrix<double>& x) {
    Matrix<double> y(x.rows(), m_coeffs.cols());
    for (unsigned j = 0; j < m_coeffs.cols(); ++j) {
        for (unsigned i = 0; i < x.rows(); ++i) {
            y(i, j) = m_coeffs(0, j);
        }
        for (unsigned k = 1; k < m_coeffs.rows(); ++k) {
            for (unsigned i = 0; i < x.rows(); ++i) {
                y(i, j) = m_coeffs(k, j) + y(i, j) * x(i, 0);
            }
        }
    }

    return y;
}

void PolynomialMap::setMap(std::set<std::string> const& in, OutMap const& outMap) {
    setIn(in);
    if (dimDomain() != 1) {
        std::ostringstream os;
        os << "Polynomial map requires 1D input (got ";
        printWithSeparator(in, os);
        os << ").";
        throw std::invalid_argument(addFileReference(os.str()));
    }

    std::set<std::string> out;
    unsigned nCoeffs = std::numeric_limits<unsigned>::max();
    for (auto const& kv : outMap) {
        out.insert(kv.first);
        if (nCoeffs != std::numeric_limits<unsigned>::max() && kv.second.size() != nCoeffs) {
            throw std::invalid_argument(addFileReference(
                "All parameters in a polynomial map must have the same number of coefficients."));
        }
        nCoeffs = kv.second.size();
    }
    setOut(out);

    m_coeffs.reallocate(nCoeffs, outMap.size());
    unsigned col = 0;
    for (auto const& kv : outMap) {
        unsigned row = 0;
        for (auto const& v : kv.second) {
            m_coeffs(row, col) = v;
            ++row;
        }
        ++col;
    }
}

} // namespace easi
