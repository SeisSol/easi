#ifndef EASI_UTIL_MATRIX_H_
#define EASI_UTIL_MATRIX_H_

#include "Slice.h"

#include <cassert>
#include <memory>

namespace easi {

template <typename T> class Matrix {
public:
    Matrix();
    Matrix(unsigned rows, unsigned cols);

    void reallocate(unsigned rows, unsigned cols);
    void clear();

    Matrix<T> block(unsigned rows, unsigned cols) const;
    Matrix<T> transposed() const;

    unsigned rows() const { return m_rows; }
    unsigned cols() const { return m_cols; }

    inline T& operator()(unsigned i, unsigned j) { return *(m_data.get() + i + j * m_ld); }
    inline T const& operator()(unsigned i, unsigned j) const {
        return *(m_data.get() + i + j * m_ld);
    }

    Slice<T> rowSlice(unsigned index) { return Slice<T>(m_ld, m_cols, m_data.get() + index); }
    Slice<T> colSlice(unsigned index) { return Slice<T>(1, m_rows, m_data.get() + index * m_ld); }

private:
    unsigned m_rows, m_cols, m_ld;

    std::shared_ptr<T> m_data;
};

template <typename T> Matrix<T>::Matrix() : m_rows(0), m_cols(0), m_ld(0), m_data(nullptr) {}

template <typename T>
Matrix<T>::Matrix(unsigned rows, unsigned cols)
    : m_rows(rows), m_cols(cols), m_ld(rows),
      m_data(new T[rows * cols], std::default_delete<T[]>()) {}

template <typename T> void Matrix<T>::reallocate(unsigned rows, unsigned cols) {
    m_rows = rows;
    m_cols = cols;
    m_ld = rows;
    m_data = std::shared_ptr<T>(new T[rows * cols], std::default_delete<T[]>());
}

template <typename T> void Matrix<T>::clear() {
    m_rows = 0;
    m_cols = 0;
    m_ld = 0;
    m_data = nullptr;
}

template <typename T> Matrix<T> Matrix<T>::block(unsigned rows, unsigned cols) const {
    if (rows <= m_rows && cols <= m_cols) {
        Matrix<T> result(*this);
        result.m_rows = rows;
        result.m_cols = cols;

        return result;
    }

    Matrix<T> result(rows, cols);
    for (unsigned j = 0; j < m_cols; ++j) {
        for (unsigned i = 0; i < m_rows; ++i) {
            result(i, j) = operator()(i, j);
        }
    }
    for (unsigned j = m_cols; j < result.m_cols; ++j) {
        for (unsigned i = m_rows; i < result.m_rows; ++i) {
            result(i, j) = static_cast<T>(0);
        }
    }
    return result;
}

template <typename T> Matrix<T> Matrix<T>::transposed() const {
    Matrix<T> result(m_cols, m_rows);
    for (unsigned j = 0; j < result.m_cols; ++j) {
        for (unsigned i = 0; i < result.m_rows; ++i) {
            result(i, j) = operator()(j, i);
        }
    }
    return result;
}

// TODO: Replace by something proper
template <typename T> Matrix<T> operator*(Matrix<T> const& A, Matrix<T> const& B) {
    assert(A.cols() == B.rows());

    Matrix<T> C(A.rows(), B.cols());
    for (unsigned j = 0; j < C.cols(); ++j) {
        for (unsigned i = 0; i < C.rows(); ++i) {
            C(i, j) = static_cast<T>(0);
            for (unsigned k = 0; k < A.cols(); ++k) {
                C(i, j) += A(i, k) * B(k, j);
            }
        }
    }

    return C;
}

} // namespace easi

#endif
