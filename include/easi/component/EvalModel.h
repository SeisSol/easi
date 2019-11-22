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
#ifndef EASI_COMPONENT_EVALMODEL_H_
#define EASI_COMPONENT_EVALMODEL_H_

#include "easi/component/Map.h"

namespace easi {
class EvalModel : public Map {
public:
  virtual ~EvalModel() { delete m_model; }

  virtual void evaluate(Query& query, ResultAdapter& result);  
  void setModel(std::set<std::string> const& in, std::set<std::string> const& out, Component* model) {
    setIn(in);
    setOut(out);
    m_model = model;
  }

protected:
  virtual Matrix<double> map(Matrix<double>& x) { return x; }

private:
  Component* m_model;
};

void EvalModel::evaluate(Query& query, ResultAdapter& result) {
  Query subQuery = query.shallowCopy();
  
  Matrix<double> y(subQuery.x.rows(), dimCodomain());
  ArraysAdapter<> adapter;
  unsigned col = 0;
  for (auto const& o : out()) {
    adapter.addBindingPoint(o, &y(0, col++));
  }
  m_model->evaluate(subQuery, adapter);
  query.x = y;
  
  Composite::evaluate(query, result);
}
}


#endif
