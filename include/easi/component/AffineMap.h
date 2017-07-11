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
  virtual ~AffineMap() {}
  
  void setMap(Matrix<double> const& matrix, Vector<double> const& translation);
  
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

void AffineMap::setMap(Matrix<double> const& matrix, Vector<double> const& translation) {
  m_matrix = matrix.transposed();
  m_translation = translation;
  
  assert(m_matrix.cols() == m_translation.size());
  
  setDimDomain(m_matrix.rows());
  setDimCodomain(m_matrix.cols());
}
}

#endif
