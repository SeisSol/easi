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
#ifndef EASI_COMPONENT_SCECFILE_H_
#define EASI_COMPONENT_SCECFILE_H_

#include <fstream>
#include "easi/component/Grid.h"
#include "easi/util/RegularGrid.h"

namespace easi {
class SCECFile : public Grid<SCECFile> {
public:
  SCECFile();
  virtual ~SCECFile() {
    delete m_grid;
  }

  void setMap(std::set<std::string> const& in, std::string const& fileName);

  void getNeighbours(Slice<double> const& x, double* weights, double* buffer) {
    m_grid->getNeighbours(x, weights, buffer);
  }
  unsigned permutation(unsigned index) const { return index; }

protected:
  virtual unsigned numberOfThreads() const { return 1; }

private:
  void readSCECFile(std::string const& fileName);
  
  RegularGrid* m_grid = nullptr;
};

SCECFile::SCECFile() {
  //                                 0      1                      2       3       4        5           6           7
  std::set<std::string> out{{"cohesion", "d_c", "forced_rupture_time", "mu_d", "mu_s", "s_dip", "s_normal", "s_strike"}};
  setOut(out);
}

void SCECFile::setMap(std::set<std::string> const& in, std::string const& fileName) {
  setIn(in);
  if (dimDomain() != 2) {
    std::ostringstream os;
    os << "SCECFile requires 2D input (got ";
    printWithSeparator(in, os);
    os << ").";
    throw std::invalid_argument(os.str());
  }
  
  readSCECFile(fileName);
}

void SCECFile::readSCECFile(std::string const& fileName) {
  m_grid = new RegularGrid;

  std::ifstream in(fileName, std::ifstream::in);
  
  if (!in.good()) {
    throw std::runtime_error("Could not open " + fileName + " for reading.");
  }

  std::string dummyLine;
  double dummyVal;
  int dummyVal2;
  unsigned numGridPoints[2];
  double min[2];
  double max[2];
  in >> dummyVal2 >> dummyVal2; // skip random seed line
  in >> numGridPoints[0] >> numGridPoints[1] >> max[0] >> max[1];
  
  for (unsigned i = 0; i < 2; ++i) {
    min[i] = 0.0;
    ++numGridPoints[i];             // number of cells -> number of grid points
  }
  in >> dummyVal2 >> dummyVal2 >> dummyVal >> dummyVal >> dummyVal >> dummyVal; // skip hypocentre line
  
  m_grid->allocate(numGridPoints, 2, dimCodomain());
  m_grid->setVolume(min, max);
  
  unsigned nGridPoints = numGridPoints[0] * numGridPoints[1];
  
  for (unsigned line = 0; line < nGridPoints; ++line) {
    unsigned idx[2];
    in >> idx[0] >> idx[1] >> dummyVal >> dummyVal;
    double* gridPtr = (*m_grid)(idx);
    in >> gridPtr[6]  // normal stress
       >> gridPtr[7]  // shear stress along strike
       >> gridPtr[5]  // shear stress along dip
       >> dummyVal    // shear stress along strike / normal stress
       >> dummyVal    // shear stress along dip    / normal stress
       >> gridPtr[4]  // static coefficient of friction
       >> gridPtr[3]  // dynamic coefficient of friction
       >> gridPtr[1]  // slip weakening critical distance
       >> gridPtr[0]  // cohesion
       >> gridPtr[2]; // time of forced rupture
  }
}
}


#endif
