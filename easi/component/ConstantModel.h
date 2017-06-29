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
#ifndef EASI_COMPONENT_CONSTANTMODEL_H_
#define EASI_COMPONENT_CONSTANTMODEL_H_

#include <map>
#include <string>
#include "easi/component/Model.h"

namespace easi {
class ConstantModel : public Model {
public:
  typedef std::map<std::string, double> Parameters;

  virtual ~ConstantModel() {}

  virtual void evaluate(Query& query, ResultAdapter& result);
  
  using Component::setDimDomain; // Make setDimDomain public
  void setParameters(Parameters const& parameters);

private:
  Parameters m_params;
};

void ConstantModel::evaluate(Query& query, ResultAdapter& result) {
  Vector<unsigned> index(1);
  Matrix<double> value(result.numberOfBindingPoints(), 1);
  unsigned numberOfMatchedBPs = 0;
  for (Parameters::const_iterator it = m_params.begin(); it != m_params.end(); ++it) {
    int bP = result.bindingPoint(it->first);
    if (bP >= 0) {
      value(bP,0) = it->second;
      ++numberOfMatchedBPs;
    }
  }
  if (numberOfMatchedBPs != result.numberOfBindingPoints()) {
    throw std::runtime_error("Parameter set required by simulation does not match model.");
  }
  
  for (unsigned j = 0; j < query.numPoints(); ++j) {
    index(0) = query.index(j);
    result.set(index, value);
  }
}

void ConstantModel::setParameters(Parameters const& parameters) {
  m_params = parameters;
  setDimCodomain(parameters.size());
}
}


#endif
