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

void model(double x, double z, double params[9]) {
  params[0]  = 0.0; // s_xx
  params[2]  = 0.0; // s_zz
  params[5]  = 0.0; // s_xz 
  params[4]  = 0.0; // s_yz
  params[1]  = -6.000000E+07; // s_yy
  params[6]  = 6.770000E-01; // mu_s
  params[7]  = 3.730000E-01; // mu_d
  params[8]  = 4.000000E-01; // d_c
  params[10] = 1.000000E+09; // forced_rupture_time
  
  z *= -1.0;
  
  if (x <= 0.0 && z <= 0.0) {
    params[3]  = 2.509795E+07; // s_xy
    params[9]  = -4.000000E+06; // cohesion
  } else if (x <= 75.0 && z <= 0.0) {
    double xi = x / 75.0;
    params[3]  = 2.509795E+07 * (1.0-xi) + 2.523141E+07 * xi; // s_xy
    params[9]  = -4.000000E+06; // cohesion
  } else if (x >= 75.0 && x <= 150.0 && z >= 150.0 && z <= 225.0) {
    double xi  = (x- 75.0) / 75.0;
    double eta = (z-150.0) / 75.0;
    params[3]  = (2.444593E+07 * (1.0-eta) + 2.445351E+07 * eta) * (1.0 - xi)
               + (2.488395E+07 * (1.0-eta) + 2.498051E+07 * eta) * xi; // s_xy
    params[9]  = -((3.800000E+06 * (1.0-eta) + 3.700000E+06 * eta) * (1.0 - xi)
               + (3.800000E+06 * (1.0-eta) + 3.700000E+06 * eta) * xi); // cohesion
  } else {
    params[3]  = 2.498051E+07; // s_xy
    params[9]  = -3.700000E+06; // cohesion
  }
}

void assertEqual(double x, double z, unsigned index, std::vector<std::vector<double>> const& material) {
  double params[11];
  model(x, z, params);
  
  for (unsigned i = 0; i < 11; ++i) {
    assert(equal(material[i][index], params[i], fabs(std::numeric_limits<double>::epsilon() * params[i])));
  }
}

int main(int argc, char** argv) {
  assert(argc == 2);

  easi::Query query = createQuery<3>({
      {1, {  -5.0, 0.0,     5.0}},
      {1, {  25.0, 0.0,     5.0}},
      {1, { 100.0, 0.0, -200.0}},
      {1, {1000.0, 0.0, -1000.0}}
    });
  std::vector<std::string> parameters{"s_xx", "s_yy", "s_zz", "s_xy", "s_yz", "s_xz", "mu_s", "mu_d", "d_c", "cohesion", "forced_rupture_time"};
  auto material = genericModel(argv[1], query, parameters);

  assertEqual(  -5.0,     5.0, 0, material);
  assertEqual(  25.0,     5.0, 1, material);
  assertEqual( 100.0,  -200.0, 2, material);
  assertEqual(1000.0, -1000.0, 3, material);

  return 0;
}
