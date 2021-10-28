#ifndef EASI_COMPONENT_ANY_H_
#define EASI_COMPONENT_ANY_H_

#include "easi/component/Filter.h"

namespace easi {

class Any : public Filter {
public:
    inline virtual ~Any() {}

    using Filter::setInOut; // Make setInOut public
    inline virtual bool accept(int, Slice<double> const&) const { return true; }
};

} // namespace easi

#endif
