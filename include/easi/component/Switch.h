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
#ifndef EASI_COMPONENT_SWITCH_H_
#define EASI_COMPONENT_SWITCH_H_

#include <set>
#include <algorithm>
#include "easi/component/Filter.h"
#include "easi/util/Print.h"

namespace easi {
class Switch : public Filter {
public:
  virtual ~Switch() {}
  
  virtual void evaluate(Query& query, ResultAdapter& result);
  virtual bool accept(int group, Slice<double> const&) const { return true; }
  virtual bool acceptAlways() const { return true; }

  virtual void add(Component* component, std::set<std::string> const& restrictions);

  using Filter::setInOut;
protected:
  using Composite::add; // Make add protected

private:
  std::vector<std::set<std::string>> m_restrictions;
  std::set<std::string> m_parameters;
};

void Switch::add(Component* component, std::set<std::string> const& restrictions) {
  std::set<std::string> overlap;
  std::set_intersection(restrictions.begin(), restrictions.end(),
                        m_parameters.begin(), m_parameters.end(),
                        std::inserter(overlap, overlap.end()));
  if (overlap.size() != 0) {
    std::stringstream s;
    s << "Switch requires a partition of parameters. The parameters {";
    printWithSeparator(overlap, s);
    s << "} were specified in multiple branches.";
    throw std::invalid_argument(
      addFileReference(s.str())
    );
  }

  Composite::add(component);
  m_restrictions.push_back(restrictions);
  for (auto const& r : restrictions) {
    m_parameters.insert(r);
  }
}

void Switch::evaluate(Query& query, ResultAdapter& result) {
  if (!result.isSubset(m_parameters)) {
    throw std::invalid_argument(
      addFileReference(
        addMissingParameters("Switch is not complete with respect to request.", result.parameters(), m_parameters)
      )
    );
  }
  
  // Evaluate submodels
  iterator c = begin();
  auto r = m_restrictions.cbegin();
  for (; c != end() && r != m_restrictions.cend(); ++c, ++r) {
    Query subQuery = query.shallowCopy();
    ResultAdapter* subResult = result.subsetAdapter(*r);
    if (subResult->numberOfParameters() > 0) {
      (*c)->evaluate(subQuery, *subResult);
    }
    delete subResult;
  }
}
}

#endif
