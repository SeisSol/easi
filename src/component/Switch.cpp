#include "easi/component/Switch.h"
#include "easi/Component.h"
#include "easi/Query.h"
#include "easi/ResultAdapter.h"
#include "easi/util/Print.h"

#include <algorithm>
#include <iterator>
#include <ostream>
#include <stdexcept>

namespace easi {

void Switch::add(Component* component, std::set<std::string> const& restrictions) {
    std::set<std::string> overlap;
    std::set_intersection(restrictions.begin(), restrictions.end(), m_parameters.begin(),
                          m_parameters.end(), std::inserter(overlap, overlap.end()));
    if (overlap.size() != 0) {
        std::stringstream s;
        s << "Switch requires a partition of parameters. The parameters {";
        printWithSeparator(overlap, s);
        s << "} were specified in multiple branches.";
        throw std::invalid_argument(addFileReference(s.str()));
    }

    Composite::add(component);
    m_restrictions.push_back(restrictions);
    for (auto const& r : restrictions) {
        m_parameters.insert(r);
    }
}

void Switch::evaluate(Query& query, ResultAdapter& result) {
    if (!result.isSubset(m_parameters)) {
        throw std::invalid_argument(addFileReference(addMissingParameters(
            "Switch is not complete with respect to request.", result.parameters(), m_parameters)));
    }

    // Evaluate submodels
    iterator c = begin();
    auto r = m_restrictions.cbegin();
    for (; c != end() && r != m_restrictions.cend(); ++c, ++r) {
        Query subQuery = query.shallowCopy();
        ResultAdapter* subResult = result.subsetAdapter(*r);
        if (subResult->numberOfParameters() > 0) {
            (*c)->evaluate(subQuery, *subResult);
        }
        delete subResult;
    }
}

} // namespace easi
