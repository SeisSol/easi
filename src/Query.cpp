#include "easi/Query.h"

namespace easi {

Query::Query(unsigned numPoints, unsigned dimDomain, bool initIndices)
    : group(numPoints), x(numPoints, dimDomain), index(numPoints) {
    if (initIndices) {
        for (unsigned i = 0; i < numPoints; ++i) {
            index(i) = i;
        }
    }
}

Query Query::shallowCopy() {
    Query copy = *this;
    return copy;
}

void Query::clear() {
    group.clear();
    x.clear();
    index.clear();
}

} // namespace easi
