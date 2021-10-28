#ifndef EASI_COMPONENT_SWITCH_H_
#define EASI_COMPONENT_SWITCH_H_

#include "easi/component/Composite.h"
#include "easi/component/Filter.h"

#include <set>
#include <string>
#include <vector>

namespace easi {

class Component;
class ResultAdapter;
struct Query;
template <typename T> class Slice;

class Switch : public Filter {
public:
    inline virtual ~Switch() {}

    virtual void evaluate(Query& query, ResultAdapter& result);
    inline virtual bool accept(int group, Slice<double> const&) const { return true; }
    inline virtual bool acceptAlways() const { return true; }

    virtual void add(Component* component, std::set<std::string> const& restrictions);

    using Filter::setInOut;

protected:
    using Composite::add; // Make add protected

private:
    std::vector<std::set<std::string>> m_restrictions;
    std::set<std::string> m_parameters;
};

} // namespace easi

#endif
