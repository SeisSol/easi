/**
 * @file
 * This file is part of SeisSol.
 *
 * @author Carsten Uphoff (c.uphoff AT tum.de, http://www5.in.tum.de/wiki/index.php/Carsten_Uphoff,_M.Sc.)
 *
 * @section LICENSE
 * Copyright (c) 2017, SeisSol Group
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @section DESCRIPTION
 **/
#ifndef EASI_UTIL_MATRIX_H_
#define EASI_UTIL_MATRIX_H_

#include <memory>
#include <cassert>

template<typename T>
class Slice {
public:
  Slice(unsigned offset, unsigned size, double* data)
    : m_offset(offset), m_size(size), m_data(data) {}
    
  inline T& operator()(unsigned i) { return m_data[i*m_offset]; }
  inline T const& operator()(unsigned i) const { return m_data[i*m_offset]; }
  inline unsigned size() const { return m_size; }
  
private:
  unsigned m_offset, m_size;
  T* m_data;
};

template<typename T>
class Matrix {
public:
  Matrix();
  Matrix(unsigned rows, unsigned cols);
  
  void reallocate(unsigned rows, unsigned cols);
  void clear();
  
  Matrix<T> block(unsigned rows, unsigned cols) const;
  Matrix<T> transposed() const;
  
  unsigned rows() const { return m_rows; }
  unsigned cols() const { return m_cols; }
  
  inline T& operator()(unsigned i, unsigned j) { return *(m_data.get() + i + j*m_ld); }
  inline T const& operator()(unsigned i, unsigned j) const { return *(m_data.get() + i + j*m_ld); }
  
  Slice<T> rowSlice(unsigned index) { return Slice<T>(m_ld, m_cols, m_data.get() + index); }
  Slice<T> colSlice(unsigned index) { return Slice<T>(1, m_rows, m_data.get() + index * m_ld); }
  
private:
  unsigned m_rows, m_cols, m_ld;
  
  std::shared_ptr<T> m_data;
};

template<typename T>
class Vector : public Matrix<T> {
public:
  Vector() : Matrix<T>() {}
  Vector(unsigned rows) : Matrix<T>(rows, 1) {}
  
  void reallocate(unsigned size) { Matrix<T>::reallocate(size, 1); }
  
  inline T& operator()(unsigned i) { return Matrix<T>::operator()(i,0); }
  inline T const& operator()(unsigned i) const { return Matrix<T>::operator()(i,0); }
  inline unsigned size() const { return Matrix<T>::rows(); }
};

template<typename T>
Matrix<T>::Matrix()
  : m_rows(0),
    m_cols(0),
    m_ld(0),
    m_data(nullptr) {
}

template<typename T>
Matrix<T>::Matrix(unsigned rows, unsigned cols)
  : m_rows(rows),
    m_cols(cols),
    m_ld(rows),
    m_data(new T[rows*cols], std::default_delete<T[]>()) {
}

template<typename T>
void Matrix<T>::reallocate(unsigned rows, unsigned cols) {
  m_rows = rows;
  m_cols = cols;
  m_ld = rows;
  m_data = std::shared_ptr<T>(new T[rows*cols], std::default_delete<T[]>());
}

template<typename T>
void Matrix<T>::clear() {
  m_rows = 0;
  m_cols = 0;
  m_ld = 0;
  m_data = nullptr;
}

template<typename T>
Matrix<T> Matrix<T>::block(unsigned rows, unsigned cols) const {
  if (rows <= m_rows && cols <= m_cols) {
    Matrix<T> result(*this);
    result.m_rows = rows;
    result.m_cols = cols;
    
    return result;
  }
  
  Matrix<T> result(rows, cols);
  for (unsigned j = 0; j < m_cols; ++j) {
    for (unsigned i = 0; i < m_rows; ++i) {
      result(i,j) = operator()(i,j);
    }
  }
  for (unsigned j = m_cols; j < result.m_cols; ++j) {
    for (unsigned i = m_rows; i < result.m_rows; ++i) {
      result(i,j) = static_cast<T>(0);
    }
  }
  return result;
}

template<typename T>
Matrix<T> Matrix<T>::transposed() const {
  Matrix<T> result(m_cols, m_rows);
  for (unsigned j = 0; j < result.m_cols; ++j) {
    for (unsigned i = 0; i < result.m_rows; ++i) {
      result(i,j) = operator()(j,i);
    }
  }
  return result;
}

// TODO: Replace by something proper
template<typename T>
Matrix<T> operator*(Matrix<T> const& A, Matrix<T> const& B) {
  assert(A.cols() == B.rows());
  
  Matrix<T> C(A.rows(), B.cols());
  for (unsigned j = 0; j < C.cols(); ++j) {
    for (unsigned i = 0; i < C.rows(); ++i) {
      C(i,j) = static_cast<T>(0);
      for (unsigned k = 0; k < A.cols(); ++k) {
        C(i,j) += A(i,k) * B(k,j);
      }
    }
  }
  
  return C;
}

#endif
