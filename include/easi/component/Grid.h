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
#ifndef EASI_COMPONENT_GRID_H_
#define EASI_COMPONENT_GRID_H_

#include "easi/component/Map.h"

namespace easi {
template<typename Derived, typename ValueType>
class Grid : public Map {
public:
  virtual ~Grid() {}

  virtual Matrix<double> map(Matrix<double>& x);

  void getNeighbours(Slice<double> const& x, double* weights, ValueType* buffer) {
    static_cast<Derived*>(this)->getNeighbours(x, weights, buffer);
  }
  unsigned permutation(unsigned index) const {
    return static_cast<Derived const*>(this)->permutation(index);
  }

protected:
  virtual unsigned numberOfThreads() const = 0;
};

template<typename GridImpl, typename ValueType>
Matrix<double> Grid<GridImpl, ValueType>::map(Matrix<double>& x) {  
  Matrix<double> y(x.rows(), dimCodomain());
#ifdef _OPENMP
  #pragma omp parallel num_threads(numberOfThreads()) shared(x,y)
#endif
  {
    ValueType* neighbours = new ValueType[(1 << dimDomain()) * dimCodomain()];
    double* weights = new double[dimDomain()];
#ifdef _OPENMP
    #pragma omp for
#endif
    for (unsigned i = 0; i < x.rows(); ++i) {
      getNeighbours(x.rowSlice(i), weights, neighbours);
      
      // linear interpolation
      for (int d = dimDomain()-1; d >= 0; --d) {
        for (int p = 0; p < (1 << d); ++p) {
          for (int v = 0; v < dimCodomain(); ++v) {
            neighbours[p*dimCodomain() + v] = neighbours[p*dimCodomain() + v] * (1.0-weights[d]) + neighbours[((1 << d) + p)*dimCodomain() + v] * weights[d];
          }
        }
      }
      
      for (int v = 0; v < dimCodomain(); ++v) {
        y(i,permutation(v)) = neighbours[v];
      }
    }
  
    delete[] weights;
    delete[] neighbours;
  }
  
  return y;
}
}


#endif
