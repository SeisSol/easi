#ifndef EASI_QUERY_H_
#define EASI_QUERY_H_

#include "easi/util/Matrix.h"
#include "easi/util/Vector.h"

namespace easi {

struct Query {
    Vector<int> group;
    Matrix<double> x;
    Vector<unsigned> index;

    Query(unsigned numPoints, unsigned dimDomain, bool initIndices = true);
    ~Query();

    inline unsigned numPoints() const { return x.rows(); }
    inline unsigned dimDomain() const { return x.cols(); }

    Query shallowCopy();
    void clear();
};

} // namespace easi

#endif
