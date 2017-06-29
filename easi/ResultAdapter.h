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
#ifndef EASI_RESULTADAPTER_H_
#define EASI_RESULTADAPTER_H_

#include "easi/util/Matrix.h"

namespace easi {
class ResultAdapter {
public:
  virtual int bindingPoint(std::string const& parameter) const = 0;
  virtual unsigned numberOfBindingPoints() const = 0;
  virtual void set(Vector<unsigned> const& index, Matrix<double> const& value) = 0;
};

template<typename T, unsigned BindingPoints>
class ArrayOfStructsAdapter : public ResultAdapter {
public:
  ArrayOfStructsAdapter(T* arrayOfStructs) : m_arrayOfStructs(arrayOfStructs) {}

  void setBindingPoint(std::string const& parameter, unsigned bindingPoint, double T::* pointerToMember) {
    m_bindingPoint[parameter] = bindingPoint;
    m_parameter[bindingPoint] = pointerToMember;
  } 

  virtual int bindingPoint(std::string const& parameter) const {
    auto it = m_bindingPoint.find(parameter);
    return (it != m_bindingPoint.end()) ? it->second : -1;
  }
  virtual unsigned numberOfBindingPoints() const { return BindingPoints; }
  
  virtual void set(Vector<unsigned> const& index, Matrix<double> const& value) {
    assert(value.rows() == BindingPoints);
    assert(value.cols() == index.size());

    for (unsigned i = 0; i < index.size(); ++i) {
      for (unsigned j = 0; j < BindingPoints; ++j) {
        m_arrayOfStructs[ index(i) ].*m_parameter[j] = value(j,i);
      }
    }
  }

private:
  T* m_arrayOfStructs;
  std::unordered_map<std::string, unsigned> m_bindingPoint;
  double T::* m_parameter[BindingPoints];
};
}

#endif
