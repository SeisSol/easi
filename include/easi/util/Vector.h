#ifndef VECTOR_20210903_H
#define VECTOR_20210903_H

#include "Matrix.h"

namespace easi {

template <typename T> class Vector : public Matrix<T> {
public:
    Vector() : Matrix<T>() {}
    Vector(unsigned rows) : Matrix<T>(rows, 1) {}

    void reallocate(unsigned size) { Matrix<T>::reallocate(size, 1); }

    inline T& operator()(unsigned i) { return Matrix<T>::operator()(i, 0); }
    inline T const& operator()(unsigned i) const { return Matrix<T>::operator()(i, 0); }
    inline unsigned size() const { return Matrix<T>::rows(); }
};

} // namespace easi

#endif // VECTOR_20210903_H
