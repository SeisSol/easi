#ifndef EASI_COMPONENT_POLYNOMIALMAP_H_
#define EASI_COMPONENT_POLYNOMIALMAP_H_

#include "easi/component/Map.h"
#include "easi/util/Matrix.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace easi {

class PolynomialMap : public Map {
public:
    using OutMap = std::map<std::string, std::vector<double>>;

    inline virtual ~PolynomialMap() {}

    void setMap(std::set<std::string> const& in, OutMap const& outMap);

protected:
    virtual Matrix<double> map(Matrix<double>& x);

private:
    Matrix<double> m_coeffs;
};

} // namespace easi

#endif
