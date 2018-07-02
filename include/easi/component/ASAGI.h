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
#ifndef EASI_COMPONENT_ASAGI_H_
#define EASI_COMPONENT_ASAGI_H_

#include <fstream>
#include "easi/component/Grid.h"
#ifdef USE_ASAGI
#include <asagi.h>
#else
namespace asagi {
  class Grid;
}
#endif

namespace easi {
class ASAGI : public Grid<ASAGI> {
public:
  static unsigned const MaxDimensions = 6;

  virtual ~ASAGI() {
    delete m_grid;
    delete[] m_permutation;
  }

  virtual bool accept(int, Slice<double> const& x) const;
  virtual bool acceptAlways() const { return false; }

  void setGrid(std::set<std::string> const& in, std::vector<std::string> const& parameters, asagi::Grid* grid, unsigned numberOfThreads);

  void getNeighbours(Slice<double> const& x, double* weights, double* buffer);
  unsigned permutation(unsigned index) const { return m_permutation[index]; }

protected:
  virtual unsigned numberOfThreads() const { return m_numberOfThreads; }

private:  
  asagi::Grid* m_grid = nullptr;
  unsigned* m_permutation = nullptr;
  unsigned m_numberOfThreads;
  unsigned m_numValues;
  
  double m_min[MaxDimensions];
  double m_max[MaxDimensions];
  double m_delta[MaxDimensions];
  double m_deltaInv[MaxDimensions];
};

bool ASAGI::accept(int, Slice<double> const& x) const {
  bool acc = true;
  for (unsigned d = 0; d < m_grid->getDimensions(); ++d) {
    acc = acc && (x(d) >= m_min[d]) && (x(d) <= m_max[d]);
  }
  return acc;
}

void ASAGI::setGrid(std::set<std::string> const& in, std::vector<std::string> const& parameters, asagi::Grid* grid, unsigned numberOfThreads) {
  setIn(in);
  if (dimDomain() != grid->getDimensions()) {
    std::ostringstream os;
    os << "ASAGI requires " << grid->getDimensions() << "D input (got ";
    printWithSeparator(in, os);
    os << ").";
    throw std::invalid_argument(addFileReference(os.str()));
  }  
  if (dimDomain() > MaxDimensions) {
    throw std::runtime_error(addFileReference("Unsupported number of dimensions for ASAGI."));
  }
  
  std::set<std::string> out;
  out.insert(parameters.begin(), parameters.end());
  setOut(out);
  m_numValues = grid->getVarSize() / sizeof(float);
  if (dimCodomain() != m_numValues) {
    std::ostringstream os;
    os << "ASAGI supplies " << m_numValues << "D output (got ";
    printWithSeparator(out, os);
    os << ").";
    throw std::invalid_argument(addFileReference(os.str()));
  }
  
  delete m_grid;
  m_grid = grid;
  m_numberOfThreads = numberOfThreads;
  
  for (unsigned d = 0; d < grid->getDimensions(); ++d) {
    m_min[d] = grid->getMin(d);
    m_max[d] = grid->getMax(d);
    m_delta[d] = grid->getDelta(d);
    m_deltaInv[d] = 1.0 / m_delta[d];
  }
  
  delete[] m_permutation;
  m_permutation = new unsigned[m_numValues];
  unsigned* perm = m_permutation;
  for (auto const& o : out) {
    auto it = std::find(parameters.begin(), parameters.end(), o);
    *perm++ = std::distance(parameters.begin(), it);
  }
}

void ASAGI::getNeighbours(Slice<double> const& x, double* weights, double* buffer) {
  double lowPos[MaxDimensions];
  for (unsigned d = 0; d < m_grid->getDimensions(); ++d) {
    lowPos[d] = m_min[d] + std::floor((x(d) - m_min[d]) * m_deltaInv[d]) * m_delta[d];
    weights[d] = (x(d) - lowPos[d]) * m_deltaInv[d];
  }

  double pos[MaxDimensions];
  for (unsigned i = 0; i < (1u << m_grid->getDimensions()); ++i) {
    float* bufferSP = reinterpret_cast<float*>(buffer + i*m_numValues);
    for (unsigned d = 0; d < m_grid->getDimensions(); ++d) {
      pos[d] = std::min(lowPos[d] + ((i & (1 << d)) >> d) * m_delta[d], m_max[d]);
    }
    m_grid->getBuf(bufferSP, pos);
    for (int j = m_numValues-1; j >= 0; --j) {
      buffer[i*m_numValues + j] = static_cast<double>(bufferSP[j]);
    }
  }
}

}


#endif
