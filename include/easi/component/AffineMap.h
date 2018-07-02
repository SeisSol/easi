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
#ifndef EASI_COMPONENT_AFFINEMAP_H_
#define EASI_COMPONENT_AFFINEMAP_H_

#include "easi/component/Map.h"

namespace easi {
class AffineMap : public Map {
public:
  typedef std::map<std::string, std::vector<double>> Transformation;
  typedef std::map<std::string, double> Translation;

  virtual ~AffineMap() {}
  
  void setMap(std::set<std::string> const& in, Transformation const& matrix, Translation const& translation);
  
protected:
  virtual Matrix<double> map(Matrix<double>& x);
  
private:
  Matrix<double> m_matrix;
  Vector<double> m_translation;
};

Matrix<double> AffineMap::map(Matrix<double>& x) {
  Matrix<double> y = x * m_matrix;
  for (unsigned i = 0; i < y.rows(); ++i) {
    for (unsigned j = 0; j < y.cols(); ++j) { 
      y(i,j) += m_translation(j);
    }
  }
  return y;
}

void AffineMap::setMap(std::set<std::string> const& in, Transformation const& matrix, Translation const& translation) {
  if (matrix.size() != translation.size()) {
    throw std::invalid_argument(addFileReference("Matrix and translation must have the same size in an affine map."));
  }
    
  setIn(in);
  
  std::set<std::string> out;
  unsigned nCoeffs = std::numeric_limits<unsigned>::max();
  for (auto const& kv : matrix) {
    out.insert(kv.first);
    if (nCoeffs != std::numeric_limits<unsigned>::max() && kv.second.size() != nCoeffs) {
      throw std::invalid_argument(addFileReference("The matrix in a affine map must have the same number of coefficients for each entry."));
    }
    nCoeffs = kv.second.size();
  }
  setOut(out);
  
  if (nCoeffs != dimDomain()) {
    throw std::invalid_argument(addFileReference("Number of matrix entries does not match number of input parameters."));
  }
  
  m_matrix.reallocate(dimDomain(), dimCodomain());
  m_translation.reallocate(dimCodomain());
  
  unsigned col = 0;
  for (auto const& kv : matrix) {
    unsigned row = 0;
    for (auto const& v : kv.second) {
      m_matrix(row, col) = v;
      ++row;
    }
    ++col;
  }
  
  unsigned row = 0;
  for (auto const& kv : translation) {
    m_translation(row++) = kv.second;
  }
}

}

#endif
