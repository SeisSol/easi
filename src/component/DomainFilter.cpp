#include "easi/component/DomainFilter.h"

#include "easi/util/Slice.h"

#include <memory>
#include <set>

namespace easi {

bool AxisAlignedCuboidalDomainFilter::accept(int, const Slice<double>& x) const {
  bool acc = true;
  for (unsigned d = 0; d < dimDomain(); ++d) {
    acc = acc && (x(d) >= m_limits[d].first) && (x(d) <= m_limits[d].second);
  }
  return acc;
}

void AxisAlignedCuboidalDomainFilter::setDomain(const Limits& limits) {
  m_limits.clear();
  std::set<std::string> inout;
  for (const auto& kv : limits) {
    inout.insert(kv.first);
    m_limits.push_back(kv.second);
  }
  setInOut(inout);
}

bool SphericalDomainFilter::accept(int, const Slice<double>& x) const {
  double distance2 = 0.0;
  for (unsigned d = 0; d < dimDomain(); ++d) {
    double diff = x(d) - m_centre(d);
    distance2 += diff * diff;
  }
  return distance2 <= m_radius2;
}

void SphericalDomainFilter::setDomain(double radius, const Centre& centre) {
  m_radius2 = radius * radius;
  m_centre.reallocate(centre.size());
  std::set<std::string> inout;
  unsigned d = 0;
  for (const auto& kv : centre) {
    inout.insert(kv.first);
    m_centre(d++) = kv.second;
  }
  setInOut(inout);
}

} // namespace easi
