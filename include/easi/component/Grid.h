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
template<typename Derived>
class Grid : public Map {
public:
  enum InterpolationType {
    Nearest,
    Linear
  };

  virtual ~Grid() {}

  virtual Matrix<double> map(Matrix<double>& x);

  void setInterpolationType(std::string const& interpolationType);
  void setInterpolationType(enum InterpolationType interpolationType) { m_interpolationType = interpolationType; }

  void getNearestNeighbour(Slice<double> const& x, double* buffer) {
    static_cast<Derived*>(this)->getNearestNeighbour(x, buffer);
  }

  void getNeighbours(Slice<double> const& x, double* weights, double* buffer) {
    static_cast<Derived*>(this)->getNeighbours(x, weights, buffer);
  }
  unsigned permutation(unsigned index) const {
    return static_cast<Derived const*>(this)->permutation(index);
  }

protected:
  virtual unsigned numberOfThreads() const = 0;

private:
  enum InterpolationType m_interpolationType = Linear;
};

template<typename GridImpl>
Matrix<double> Grid<GridImpl>::map(Matrix<double>& x) {  
  Matrix<double> y(x.rows(), dimCodomain());
#ifdef _OPENMP
  #pragma omp parallel num_threads(numberOfThreads()) shared(x,y)
#endif
  {
    int interpolation_order = 5; //TODO make me a variable
    int num_interpolation_nodes = interpolation_order + 1;
    int num_interpolation_nodes_global = std::pow(num_interpolation_nodes , dimDomain())

    double* neighbours = new double[num_interpolation_nodes * dimCodomain()];
    double* weights    = new double[num_interpolation_nodes * dimDomain()  ];
    std::vector<double> lagrange_factors;
    lagrange_factors.resize(dimCodomain() * num_interpolation_nodes);

    
#ifdef _OPENMP
    #pragma omp for
#endif
    for (unsigned i = 0; i < x.rows(); ++i) {
      // TODO: interpolation of arbitrary order
      if (m_interpolationType == Linear) {
        getNeighbours(x.rowSlice(i), num_interpolation_nodes, weights, neighbours);
        
        //for each dimension reduce multindeces by one order
        for (int d = static_cast<int>(dimDomain())-1; d >= 0; --d) {
          
          int index_polynomial = 0; //1d index of the current goal multiindex
          
          //stop if multindex is out of range
          while(index_polynomial < num_interpolation_nodes_global){
            //reduction:
            std::fill_n(lagrange_factors.data(),1.0,dimCodomain());
            
            //sum over interpolation polynomials
            for(int p = 0 ; p < num_interpolation_nodes; ++p){
              //multiplication over lagrange factors       
              for(int f = 0 ; f < num_interpolation_nodes; ++f){
                if(p==f) continue;
                
                int index_factor = (index_polynomial + f) * offset;
                //interpolation for each variable
                for (int v = 0; v < static_cast<int>(dimCodomain()); ++v) {
                  lagrange_factor[p * dimCodomain() +  v] *=
                    neighbours[ index_factor * dimCodomain() + v ] *
                   (weights   [ index_factor * dimDomain()   + d ]) / ((p-f) * dx);
                }
              }
            }
            for (int v = 0; v < static_cast<int>(dimCodomain()); ++v) {
              neighbours[index_polynomial * dimCodomain() + v] = 0;
            }
            for(int p = 0 ; p < num_interpolation_nodes; ++p){
              for (int v = 0; v < static_cast<int>(dimCodomain()); ++v) {
                neighbours[index_polynomial * dimCodomain() + v] += lagrange_factor[p * dimCodomain() +  v];
              }
            }
            index_polynomial = index_polynomial + num_interpolation_nodes * offset;
          }
          offset = offset*num_interpolation_nodes; //set offset for next dimension
        }
      }else{
        getNearestNeighbour(x.rowSlice(i), neighbours);
      }
      
      for (int v = 0; v < static_cast<int>(dimCodomain()); ++v) {
        y(i,permutation(v)) = neighbours[v];
      }
    }
  
    delete[] weights;
    delete[] neighbours;
  }
  
  return y;
}

template<typename GridImpl>
void Grid<GridImpl>::setInterpolationType(std::string const& interpolationType) {
  std::string iType = interpolationType;
  std::transform(iType.begin(), iType.end(), iType.begin(), ::tolower);
  if (iType == "nearest") {
    setInterpolationType(Nearest);
  } else if (iType == "linear") {
    setInterpolationType(Linear);
  } else {
    std::stringstream ss;
    ss << "Invalid interpolation type " << interpolationType << ".";
    throw std::invalid_argument(ss.str());
  }
}
}


#endif
