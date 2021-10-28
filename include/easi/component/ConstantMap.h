#ifndef EASI_COMPONENT_CONSTANTMAP_H_
#define EASI_COMPONENT_CONSTANTMAP_H_

#include "easi/Component.h"
#include "easi/component/Map.h"
#include "easi/util/Matrix.h"

#include <map>
#include <string>
#include <vector>

namespace easi {
class ConstantMap : public Map {
public:
    typedef std::map<std::string, double> OutMap;

    virtual ~ConstantMap() {}

    virtual Matrix<double> map(Matrix<double>& x);

    using Component::setIn; // Make setIn public
    void setMap(OutMap const& outMap);

private:
    std::vector<double> m_values;
};

} // namespace easi

#endif
