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

void model(double z, double params[9]) {
  params[0] = 2670.0;
  params[1] = 32038120320.0;
  params[2] = 32043759360.0;

  double b11 = 0.926793;
  double b33 = 1.073206;
  double b13 = -0.169029;
  
  double Pf = 9800.0* fabs(z);
  double omega;
  if (z >= -15000.0) {
    omega = 1.0;
  } else if (z <= -15000.0 && z >= -20000.0) {
    omega = (20000.0-fabs(z)) / 5000.0;
  } else {
    omega = 0.0;
  }
  
  double* stress = params + 3;
  
  stress[2] = -2670.0 * 9.8 * fabs(z);
  stress[0] = omega*(b11*(stress[2]+Pf)-Pf)+(1.0-omega)*stress[2];
  stress[1] = omega*(b33*(stress[2]+Pf)-Pf)+(1.0-omega)*stress[2];
  stress[3] = omega*(b13*(stress[2]+Pf));
  stress[4] = 0.0;
  stress[5] = 0.0;

  stress[0] += Pf;
  stress[1] += Pf;
  stress[2] += Pf;
}

void assertEqual(double z, unsigned index, std::vector<std::vector<double>> const& material) {
  double params[9];
  model(z, params);
  
  for (unsigned i = 0; i < 9; ++i) {
    assert(equal(material[i][index], params[i]));
  }
}

int main(int argc, char** argv) {
  assert(argc == 2);

  easi::Query query = createQuery<3>({
      {1, { 3.0,      0.0,  -20000.0}},
      {1, {-5.0,   -100.0,  -16000.0}},
      {1, { 9.0,  -1000.0,  100000.0}},
      {1, { 9.0,  -1000.0, -100000.0}}
    });
  
  std::vector<std::string> parameters{"rho", "mu", "lambda", "s_xx", "s_yy", "s_zz", "s_xy", "s_yz", "s_xz"};
  auto material = genericModel(argv[1], query, parameters);

  assertEqual(-20000.0, 0, material);
  assertEqual(-16000.0, 1, material);
  assertEqual( 100000.0, 2, material);
  assertEqual(-100000.0, 3, material);

  return 0;
}
