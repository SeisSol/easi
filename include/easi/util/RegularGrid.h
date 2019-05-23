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
#ifndef EASI_UTIL_REGULARGRID_H_
#define EASI_UTIL_REGULARGRID_H_

#include <cmath>
#include "Matrix.h"

namespace easi {
class RegularGrid {
public:
  static unsigned const MaxDimensions = 6;

  ~RegularGrid() {
    delete[] m_values;
  }

  void allocate(unsigned const* numGridPoints, unsigned dimensions, unsigned numValues);
  void setVolume(double const* min, double const* max);
  double* operator()(unsigned const* index);
  
  void getNearestNeighbour(Slice<double> const& x, double* buffer);
  void getNeighbours(Slice<double> const& x, double* weights, double* buffer);

private:
  double*  m_values = nullptr;
  unsigned m_dimensions = 0;
  unsigned m_numValues = 0;
  double   m_min[MaxDimensions];
  double   m_max[MaxDimensions];
  unsigned m_num[MaxDimensions];
  double   m_delta[MaxDimensions];
};

void RegularGrid::allocate(unsigned const* numGridPoints, unsigned dimensions, unsigned numValues) {
  if (dimensions > MaxDimensions) {
    throw std::runtime_error("Unsupported number of dimensions for RegularGrid.");
  }
  m_dimensions = dimensions;
  m_numValues = numValues;
  unsigned size = m_numValues;
  for (unsigned d = 0; d < m_dimensions; ++d) {
    m_num[d] = numGridPoints[d];
    size *= m_num[d];
  }
  m_values = new double[size];
}

void RegularGrid::setVolume(double const* min, double const* max) {
  for (unsigned d = 0; d < m_dimensions; ++d) {
    m_min[d] = min[d];
    m_max[d] = max[d];
    m_delta[d] = (max[d]-min[d]) / (m_num[d]-1);
  }
}

double* RegularGrid::operator()(unsigned const* index) {
  unsigned stride = 1;
  unsigned idx = 0;
  for (unsigned d = 0; d < m_dimensions; ++d) {
    idx += index[d] * stride;
    stride *= m_num[d];
  }
  return m_values + m_numValues * idx;
}

void RegularGrid::getNearestNeighbour(Slice<double> const& x, double* buffer) {
  assert(x.size() == m_dimensions);

  unsigned idx[MaxDimensions];
  for (unsigned d = 0; d < m_dimensions; ++d) {
    if (x(d) < m_min[d]) {
      idx[d] = 0;
    } else if (x(d) >= m_max[d]) {
      idx[d] = m_num[d]-1;
    } else {
      double xn = (x(d) - m_min[d]) / m_delta[d];
      idx[d] = std::round(xn);
    }
  }

  double* values = operator()(idx);
  for (unsigned v = 0; v < m_numValues; ++v) {
    buffer[v] = values[v];
  }
}

void RegularGrid::getNeighbours(Slice<double> const& x, double* weights, double* buffer) {
  assert(x.size() == m_dimensions);
  
  unsigned idxBase[MaxDimensions];
  for (unsigned d = 0; d < m_dimensions; ++d) {
    if (x(d) < m_min[d]) {
      idxBase[d] = 0;
      weights[d] = 0.0;
    } else if (x(d) >= m_max[d]) {
      idxBase[d] = m_num[d]-1;
      weights[d] = 1.0;
    } else {
      double xn = (x(d) - m_min[d]) / m_delta[d];
      idxBase[d] = xn;
      weights[d] = xn - idxBase[d];
    }
  }

  unsigned idx[MaxDimensions];
  for (unsigned i = 0; i < (1u << m_dimensions); ++i) {
    for (unsigned d = 0; d < m_dimensions; ++d) {
      idx[d] = std::min(idxBase[d] + ((i & (1 << d)) >> d), m_num[d]-1);
    }
    double* values = operator()(idx);
    for (unsigned v = 0; v < m_numValues; ++v) {
      buffer[i*m_numValues + v] = values[v];
    }
  }
}
}

#endif
