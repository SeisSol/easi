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
#ifndef EASI_COMPONENT_COMPOSITE_H_
#define EASI_COMPONENT_COMPOSITE_H_

#include <vector>
#include <algorithm>
#include <numeric>
#include <sstream>
#include "easi/Component.h"
#include "easi/util/Print.h"

namespace easi {
class Composite : public Component {
public:
  typedef typename std::vector<Component*>::iterator iterator;
  typedef typename std::vector<Component*>::const_iterator const_iterator;

  virtual ~Composite();
  
  virtual void add(Component* component);

  inline virtual iterator begin() { return m_components.begin(); }
  inline virtual const_iterator cbegin() const { return m_components.cbegin(); }
  
  inline virtual iterator end() { return m_components.end(); }
  inline virtual const_iterator cend() const { return m_components.cend(); }
  
  virtual void evaluate(Query& query, ResultAdapter& result);
  
  std::string addMissingParameters(std::string const& what_arg, std::set<std::string> expected, std::set<std::string> supplied);

  virtual std::set<std::string> suppliedParameters();
  
protected:
  virtual Matrix<double> map(Matrix<double>& x) = 0;
  
private:
  void couldNotFindModelError(int group, Slice<double> const& y);
  std::vector<Component*> m_components;
};
}

#endif
