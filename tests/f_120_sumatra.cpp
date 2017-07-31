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

#include <cmath>
#include "easitest.h"

void model(double z, double params[1]) {
  double const g = 9.8;
  double layers[] = {0.0,   -2000.0, -6000.0, -12000.0, -23000.0, -600.0e6};
  double rho[] = {1000.0, 2720.0, 2860.0, 3050.0, 3300.0, 3375.0};
  double sigzz = 0.0;
  for (unsigned i = 1; i < sizeof(layers)/sizeof(double); ++i) {
    if (z > layers[i]) {
      sigzz += rho[i-1]*(z-layers[i-1])*g;
      break;
    } else {
      sigzz += rho[i-1]*(layers[i]-layers[i-1])*g;
    }
  }
  double alpha = 0.0;
  if (z >= -5000.0) {
    alpha = 0.0;
  } else {
    if (-z >= -10000.0) {
      alpha = -1.0-z/5000.0;
    } else {
      alpha = 1.0;
    }
  }
  double Pf = -1000.0 * 9.8 * z * (1.0 + alpha);
  params[0] = Pf + sigzz;
}

void assertEqual(double z, unsigned index, std::vector<std::vector<double>> const& material) {
  double params[1];
  model(z, params);
  
  for (unsigned i = 0; i < 1; ++i) {
    assert(equal(material[i][index], params[i], 10.0 * fabs(std::numeric_limits<double>::epsilon() * params[i])));
  }
}

int main(int argc, char** argv) {
  assert(argc == 2);

  easi::Query query = createQuery<3>({
      {1, { 0.0, 0.0, -1400.0}},
      {1, { 0.0, 0.0, -50000.0}},
      {1, { 0.0, 0.0, -49000.0}},
      {1, { 0.0, 0.0, -3000.0}},
      {1, { 0.0, 0.0, -22000.0}},
      {1, { 0.0, 0.0,  0.0}}
    });
  std::vector<std::string> parameters{"s_zz"};
  auto material = genericModel(argv[1], query, parameters);

  unsigned i = 0;
  assertEqual( -1400.0, i++, material);
  assertEqual(-50000.0, i++, material);
  assertEqual(-49000.0, i++, material);
  assertEqual( -3000.0, i++, material);
  assertEqual(-22000.0, i++, material);
  assertEqual(     0.0, i++, material);

  return 0;
}
