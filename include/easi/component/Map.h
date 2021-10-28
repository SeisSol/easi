#ifndef EASI_COMPONENT_MAP_H_
#define EASI_COMPONENT_MAP_H_

#include "easi/component/Composite.h"

namespace easi {

class Map : public Composite {
public:
    virtual ~Map() {}

    inline virtual bool accept(int, Slice<double> const&) const { return true; }
    inline virtual bool acceptAlways() const { return true; }
};

} // namespace easi

#endif
