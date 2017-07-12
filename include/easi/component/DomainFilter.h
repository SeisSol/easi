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
#ifndef EASI_COMPONENT_DOMAINFILTER_H_
#define EASI_COMPONENT_DOMAINFILTER_H_

#include "easi/component/Filter.h"

namespace easi {
class AxisAlignedCuboidalDomainFilter : public Filter {
public:
  typedef std::map<std::string, std::pair<double, double>> Limits;

  virtual bool accept(int, Slice<double> const& x) const;
  void setDomain(Limits const& limits);

private:
  std::vector<std::pair<double,double>> m_limits;
};

bool AxisAlignedCuboidalDomainFilter::accept(int, Slice<double> const& x) const {
  bool acc = true;
  for (unsigned d = 0; d < dimDomain(); ++d) {
    acc = acc && (x(d) >= m_limits[d].first) && (x(d) <= m_limits[d].second);
  }
  return acc;
}

void AxisAlignedCuboidalDomainFilter::setDomain(Limits const& limits) {
  m_limits.clear();
  std::set<std::string> inout;
  for (auto const& kv : limits) {
    inout.insert(kv.first);
    m_limits.push_back(kv.second);
  }
  setInOut(inout);
}

class SphericalDomainFilter : public Filter {
public:
  typedef std::map<std::string, double> Centre;
  
  virtual bool accept(int, Slice<double> const& x) const;
  void setDomain(double radius, Centre const& centre);

private:
  double m_radius2;
  Vector<double> m_centre;
};

bool SphericalDomainFilter::accept(int, Slice<double> const& x) const {
  double distance2;
  for (unsigned d = 0; d < dimDomain(); ++d) {
    double diff = x(d)-m_centre(d);
    distance2 += diff*diff;
  }
  return distance2 <= m_radius2;
}

void SphericalDomainFilter::setDomain(double radius, Centre const& centre) {
  m_radius2 = radius*radius;
  m_centre.reallocate(centre.size());
  std::set<std::string> inout;
  unsigned d = 0;
  for (auto const& kv : centre) {
    inout.insert(kv.first);
    m_centre(d++) = kv.second;
  }
  setInOut(inout);
}
}

#endif
