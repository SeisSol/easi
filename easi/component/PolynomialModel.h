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
#ifndef EASI_COMPONENT_POLYNOMIALMODEL_H_
#define EASI_COMPONENT_POLYNOMIALMODEL_H_

#include <vector>
#include <string>
#include "easi/component/Model.h"

namespace easi {
class PolynomialModel : public Model {
public:
  typedef std::vector<std::string> Parameters;

  // Currently limited to 1D domain
  PolynomialModel() { setDimDomain(1); }
  virtual ~PolynomialModel() {}

  virtual void evaluate(Query& query, ResultAdapter& result);
  
  void setCoefficients(Matrix<double> const& coeffs);
  void setParameters(Parameters const& parameters) { m_params = parameters; }

private:
  Matrix<double> m_coeffs;
  Parameters m_params;
};

// Implements Horner's method
void PolynomialModel::evaluate(Query& query, ResultAdapter& result) {
  assert(m_params.size() == dimCodomain());
  
  unsigned numberOfMatchedBPs = 0;
  Matrix<double> coeffs(result.numberOfBindingPoints(), m_coeffs.cols());
  for (unsigned i = 0; i < m_params.size(); ++i) {
    int bP = result.bindingPoint(m_params[i]);
    if (bP >= 0) {
      ++numberOfMatchedBPs;
      for (unsigned j = 0; j < m_coeffs.cols(); ++j) {
        coeffs(bP,j) = m_coeffs(i,j);
      }
    }
  }
  if (numberOfMatchedBPs != result.numberOfBindingPoints()) {
    throw std::runtime_error("Parameter set required by simulation does not match model.");
  }

  Matrix<double> Y(coeffs.rows(), query.numPoints());
  for (unsigned j = 0; j < query.numPoints(); ++j) {
    for (unsigned i = 0; i < coeffs.rows(); ++i) {
      Y(i,j) = coeffs(i,0);
    }
    for (unsigned k = 1; k < coeffs.cols(); ++k) {
      for (unsigned i = 0; i < coeffs.rows(); ++i) {
        Y(i,j) = coeffs(i,k) + Y(i,j) * query.x(j,0);
      }
    }
  }

  result.set(query.index, Y);
}


void PolynomialModel::setCoefficients(Matrix<double> const& coeffs) {
  m_coeffs = coeffs;
  setDimCodomain(m_coeffs.rows());
}
}

#endif
