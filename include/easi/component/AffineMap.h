#ifndef EASI_COMPONENT_AFFINEMAP_H_
#define EASI_COMPONENT_AFFINEMAP_H_

#include "easi/component/Map.h"
#include "easi/util/Matrix.h"
#include "easi/util/Vector.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace easi {
class AffineMap : public Map {
public:
    typedef std::map<std::string, std::vector<double>> Transformation;
    typedef std::map<std::string, double> Translation;

    inline virtual ~AffineMap() {}

    void setMap(std::set<std::string> const& in, Transformation const& matrix,
                Translation const& translation);

protected:
    virtual Matrix<double> map(Matrix<double>& x);

private:
    Matrix<double> m_matrix;
    Vector<double> m_translation;
};

} // namespace easi

#endif
