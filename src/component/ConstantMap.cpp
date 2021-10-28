#include "easi/component/ConstantMap.h"

#include <cassert>
#include <set>
#include <utility>

namespace easi {

Matrix<double> ConstantMap::map(Matrix<double>& x) {
    Matrix<double> y(x.rows(), dimCodomain());
    auto j = 0;
    for (auto const& value : m_values) {
        for (unsigned i = 0; i < x.rows(); ++i) {
            y(i, j) = value;
        }
        ++j;
    }

    return y;
}

void ConstantMap::setMap(OutMap const& outMap) {
    m_values.clear();
    std::set<std::string> out;
    for (auto const& kv : outMap) {
        out.insert(kv.first);
        m_values.push_back(kv.second);
    }
    setOut(out);

    assert(dimCodomain() == m_values.size());
}

} // namespace easi
