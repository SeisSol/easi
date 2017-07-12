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
#ifndef EASI_COMPONENT_POLYNOMIALMAP_H_
#define EASI_COMPONENT_POLYNOMIALMAP_H_

#include <vector>
#include <string>
#include "easi/component/Map.h"
#include "easi/util/Print.h"

namespace easi {
class PolynomialMap : public Map {
public:
  typedef std::map<std::string, std::vector<double>> OutMap;

  virtual ~PolynomialMap() {}

  void setMap(std::set<std::string> const& in, OutMap const& outMap);

protected:
  virtual Matrix<double> map(Matrix<double>& x);

private:
  Matrix<double> m_coeffs;
};

// Implements Horner's method
Matrix<double> PolynomialMap::map(Matrix<double>& x) {
  Matrix<double> y(x.rows(), m_coeffs.cols());
  for (unsigned j = 0; j < m_coeffs.cols(); ++j) {
    for (unsigned i = 0; i < x.rows(); ++i) {
      y(i,j) = m_coeffs(0,j);
    }
    for (unsigned k = 1; k < m_coeffs.rows(); ++k) {
      for (unsigned i = 0; i < x.rows(); ++i) {
        y(i,j) = m_coeffs(k,j) + y(i,j) * x(i,0);
      }
    }
  }

  return y;
}


void PolynomialMap::setMap(std::set<std::string> const& in, OutMap const& outMap) {
  setIn(in);
  if (dimDomain() != 1) {
    std::ostringstream os;
    os << "Polynomial map requires 1D input (got ";
    printWithSeparator(in, os);
    os << ").";
    throw std::invalid_argument(os.str());
  }

  std::set<std::string> out;
  int nCoeffs = -1;
  for (auto const& kv : outMap) {
    out.insert(kv.first);
    if (nCoeffs != -1 && kv.second.size() != nCoeffs) {
      throw std::invalid_argument("All parameters in a polynomial map must have the same number of coefficients.");
    }
    nCoeffs = kv.second.size();
  }
  setOut(out);
  
  m_coeffs.reallocate(nCoeffs, outMap.size());
  unsigned col = 0;
  for (auto const& kv : outMap) {
    unsigned row = 0;
    for (auto const& v : kv.second) {
      m_coeffs(row, col) = v;
      ++row;
    }
    ++col;
  }
}
}

#endif
