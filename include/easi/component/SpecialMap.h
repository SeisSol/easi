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
#ifndef EASI_COMPONENT_SPECIALMAP_H_
#define EASI_COMPONENT_SPECIALMAP_H_

#include "easi/component/Map.h"
#include "easi/util/MagicStruct.h"

namespace easi {
template<typename Special>
class SpecialMap : public Map {
public:
  virtual ~SpecialMap() {}

  void setMap(std::map<std::string, double> const& constants);
  
protected:
  virtual Matrix<double> map(Matrix<double>& x);

private:
  int m_inBPs[get_number_of_members<typename Special::in>()];
  int m_outBPs[get_number_of_members<typename Special::out>()];
  
  Special m_prototype;
};

template<typename Special>
Matrix<double> SpecialMap<Special>::map(Matrix<double>& x) {
  assert(x.cols() == dimDomain());
  
  Special special = m_prototype;
  double* input[get_number_of_members<typename Special::in>()];
  double* output[get_number_of_members<typename Special::out>()];
  for (unsigned j = 0; j < dimDomain(); ++j) {
    input[j] = &(special.i.*get_pointer_to_member<typename Special::in>(m_inBPs[j]));
  }
  for (unsigned j = 0; j < dimCodomain(); ++j) {
    output[j] = &(special.o.*get_pointer_to_member<typename Special::out>(m_outBPs[j]));
  }

  Matrix<double> y(x.rows(), dimCodomain());
  for (unsigned i = 0; i < y.rows(); ++i) {
    for (unsigned j = 0; j < x.cols(); ++j) {
      *(input[j]) = x(i,j);
    }
    special.evaluate();
    for (unsigned j = 0; j < y.cols(); ++j) {
      y(i,j) = *(output[j]);
    }
  }
  return y;
  return Matrix<double>();
}

template<typename Special>
void SpecialMap<Special>::setMap(std::map<std::string, double> const& constants) {
  setOut(memberNamesToSet<typename Special::out>());
  
  std::set<std::string> in = memberNamesToSet<typename Special::in>();
  for (auto const& kv : constants) {
    int bp = get_binding_point<typename Special::in>(kv.first);
    if (bp < 0) {
      std::stringstream ss;
      ss << "Unknown constant " << kv.first << ".";
      throw std::invalid_argument(ss.str());
    }
    m_prototype.i.*get_pointer_to_member<typename Special::in>(bp) = kv.second;
    in.erase(kv.first);
  }
  
  setIn(in);
  
  unsigned d = 0;
  for (auto const& i : in) {
    int bp = get_binding_point<typename Special::in>(i);
    m_inBPs[d++] = bp;
  }
  
  d = 0;
  for (auto const& o : out()) {
    int bp = get_binding_point<typename Special::out>(o);
    m_outBPs[d++] = bp;
  }
}
}

#endif
