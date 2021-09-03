#ifndef EASI_COMPONENT_DOMAINFILTER_H_
#define EASI_COMPONENT_DOMAINFILTER_H_

#include "easi/component/Filter.h"
#include "easi/util/Vector.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace easi {

template <typename T> class Slice;

class AxisAlignedCuboidalDomainFilter : public Filter {
public:
    using Limits = std::map<std::string, std::pair<double, double>>;

    virtual bool accept(int, Slice<double> const& x) const;
    void setDomain(Limits const& limits);

private:
    std::vector<std::pair<double, double>> m_limits;
};

class SphericalDomainFilter : public Filter {
public:
    using Centre = std::map<std::string, double>;

    virtual bool accept(int, Slice<double> const& x) const;
    void setDomain(double radius, Centre const& centre);

private:
    double m_radius2;
    Vector<double> m_centre;
};

} // namespace easi

#endif
