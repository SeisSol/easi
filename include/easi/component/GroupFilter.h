#ifndef EASI_COMPONENT_GROUPFILTER_H_
#define EASI_COMPONENT_GROUPFILTER_H_

#include "easi/component/Filter.h"
#include "easi/util/Slice.h"

#include <set>

namespace easi {

class GroupFilter : public Filter {
  public:
  inline virtual ~GroupFilter() {}

  virtual bool accept(int group, const Slice<double>&) const {
    return m_groups.find(group) != m_groups.end();
  }

  using Filter::setInOut; // Make setInOut public
  inline void setGroups(const std::set<int>& groups) { m_groups = groups; }

  private:
  std::set<int> m_groups;
};

} // namespace easi

#endif
