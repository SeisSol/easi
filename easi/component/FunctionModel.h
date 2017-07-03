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
#ifndef EASI_COMPONENT_FUNCTIONMODEL_H_
#define EASI_COMPONENT_FUNCTIONMODEL_H_

#include <map>
#include <string>
#include <impalajit.hh>
#include "easi/component/Model.h"

namespace easi {
class FunctionModel : public Model {
public:
  typedef std::map<std::string, std::string> Parameters;

  virtual ~FunctionModel() {}

  virtual void evaluate(Query& query, ResultAdapter& result);

  void setParameters(Parameters const& functionMap);

private:
  std::unordered_map<std::string, dasm_gen_func> m_functions;
};

typedef double (*wrapper)(dasm_gen_func, Matrix<double> const&, unsigned);

template<unsigned N>
double stupidWrapper(dasm_gen_func f, Matrix<double> const& x, unsigned index);

template<>
double stupidWrapper<1>(dasm_gen_func f, Matrix<double> const& x, unsigned index) {
  return f(x(index, 0));
}
template<>
double stupidWrapper<2>(dasm_gen_func f, Matrix<double> const& x, unsigned index) {
  return f(x(index, 0), x(index, 1));
}
template<>
double stupidWrapper<3>(dasm_gen_func f, Matrix<double> const& x, unsigned index) {
  return f(x(index, 0), x(index, 1), x(index, 2));
}

void FunctionModel::evaluate(Query& query, ResultAdapter& result) {
  wrapper w;
  switch (dimDomain()) {
    case 1: w = &stupidWrapper<1>; break;
    case 2: w = &stupidWrapper<2>; break;
    case 3: w = &stupidWrapper<3>; break;
    default: throw std::runtime_error("Unsupported number of parameters."); break; 
  }
  dasm_gen_func* functions = new dasm_gen_func[result.numberOfBindingPoints()];
  unsigned numberOfMatchedBPs = 0;
  for (auto it = m_functions.cbegin(); it != m_functions.cend(); ++it) {
    int bP = result.bindingPoint(it->first);
    if (bP >= 0) {
      functions[bP] = it->second;
      ++numberOfMatchedBPs;
    }
  }
  if (numberOfMatchedBPs != result.numberOfBindingPoints()) {
    throw std::runtime_error("Parameter set required by simulation does not match model.");
  }
  Matrix<double> Y(result.numberOfBindingPoints(), query.numPoints());
  for (unsigned j = 0; j < Y.cols(); ++j) {
    for (unsigned i = 0; i < Y.rows(); ++i) {
      Y(i,j) = (*w)(functions[i], query.x, j);
    }
  }
  delete functions;

  result.set(query.index, Y);
}

void FunctionModel::setParameters(Parameters const& functionMap) {
  setDimCodomain(functionMap.size());

  std::vector<std::string> functionDefinitions;
  for (auto it = functionMap.cbegin(); it != functionMap.cend(); ++it ) {
    functionDefinitions.push_back(it->first + it->second);
    m_functions[it->first] = nullptr;
  }
  impalajit::Compiler compiler(functionDefinitions);
  compiler.compile();
  
  unsigned dimDomain = std::numeric_limits<unsigned>::max();
  for (auto it = m_functions.begin(); it != m_functions.end(); ++it ) {
    it->second = compiler.getFunction(it->first);
    unsigned funDomain = compiler.getParameterCount(it->first);
    if (dimDomain != funDomain && dimDomain != std::numeric_limits<unsigned>::max()) {
      throw std::invalid_argument("All functions in a FunctionModel must have the same domain.");
    }
    dimDomain = funDomain;
  }
  setDimDomain(dimDomain);
}
}


#endif
