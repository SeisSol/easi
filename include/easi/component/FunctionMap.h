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
#ifndef EASI_COMPONENT_FUNCTIONMAP_H_
#define EASI_COMPONENT_FUNCTIONMAP_H_

#include "easi/component/Map.h"

namespace easi {
class FunctionMap : public Map {
public:
  typedef std::map<std::string, std::string> Parameters;

  virtual ~FunctionMap() {}

  void setMap(Parameters const& functionMap);
  
protected:
  virtual Matrix<double> map(Matrix<double>& x);

private:
  std::vector<dasm_gen_func> m_functions;
};

Matrix<double> FunctionMap::map(Matrix<double>& x) {
  assert(x.cols() == dimDomain());
  assert(m_functions.size() == dimCodomain());
  
  function_wrapper_t w = getFunctionWrapper(dimDomain());

  Matrix<double> y(x.rows(), dimCodomain());
  for (unsigned i = 0; i < y.rows(); ++i) {
    for (unsigned j = 0; j < y.cols(); ++j) {
      y(i,j) = w(m_functions[j], x, i);
    }
  }
  return y;
}

void FunctionMap::setMap(Parameters const& functionMap) {
  setDimCodomain(functionMap.size());

  std::vector<std::string> functionDefinitions;
  for (auto it = functionMap.cbegin(); it != functionMap.cend(); ++it ) {
    functionDefinitions.push_back(it->first + it->second);
  }
  impalajit::Compiler compiler(functionDefinitions);
  compiler.compile();
  
  unsigned dimDomain = std::numeric_limits<unsigned>::max();
  for (auto it = functionMap.begin(); it != functionMap.end(); ++it ) {
    m_functions.push_back(compiler.getFunction(it->first));
    unsigned funDomain = compiler.getParameterCount(it->first);
    if (dimDomain != funDomain && dimDomain != std::numeric_limits<unsigned>::max()) {
      throw std::invalid_argument("All functions in a FunctionMap must have the same domain.");
    }
    dimDomain = funDomain;
  }
  setDimDomain(dimDomain);
}
}

#endif
