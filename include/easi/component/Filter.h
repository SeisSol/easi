#ifndef EASI_COMPONENT_FILTER_H_
#define EASI_COMPONENT_FILTER_H_

#include "easi/component/Composite.h"

namespace easi {

class Filter : public Composite {
public:
    inline virtual ~Filter() {}

    inline virtual bool acceptAlways() const { return false; }

protected:
    inline void setInOut(std::set<std::string> const& inout) {
        setIn(inout);
        setOut(inout);
    }

    inline virtual Matrix<double> map(Matrix<double>& x) { return x; }
};

} // namespace easi

#endif
