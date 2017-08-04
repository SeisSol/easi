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
#include "easi/component/Special.h"

void model(double x, double y, double z, double params[6]) {
  double xS1 = 5.0000000000e+05;
  double yS1 = 4.4212739025e+05;
  double xS2 = 4.4461626476e+05;
  double yS2 = 6.0795713230e+05;
  double strike;
  if (y-yS1 < x-xS1) {
    strike = 309.0;
  } else if (y-yS2 < x-xS2) {
    double alpha = ((y-x)-(yS1-xS1)) / ((yS2-xS2)-(yS1-xS1));
    strike = (1.0-alpha)*309.0+alpha*330.0;
  } else {
    strike = 330.0;
  }
  
  double bii[6];
  STRESS_STR_DIP_SLIP_AM(0.25, 0.3, strike, 8.0, 555562000.0, 0.4e6, 0.7, 1.0, 0.5, bii);
  
  easi::STRESS_STR_DIP_SLIP_AM test;
  test.i.mu_d = 0.25;
  test.i.mu_s = 0.3;
  test.i.strike = strike;
  test.i.dip = 8.0;
  test.i.s_zz = 555562000.0;
  test.i.cohesion = 0.4e6;
  test.i.R = 0.7;
  test.i.DipSlipFaulting = 1.0;
  test.i.s2ratio = 0.5;
  test.evaluate();
  assert(equal(test.o.b_xx, bii[0]));
  assert(equal(test.o.b_yy, bii[1]));
  assert(equal(test.o.b_zz, bii[2]));
  assert(equal(test.o.b_xy, bii[3]));
  assert(equal(test.o.b_yz, bii[4]));
  assert(equal(test.o.b_xz, bii[5]));
  
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
  } else if (z >= -10000.0) {
    alpha = -1.0-z/5000.0;
  } else {
    alpha = 1.0;
  }
  double Pf = -1000.0 * 9.8 * z * (1.0 + alpha);
  
  double Rz = 0.0;
  if (z < -25000.0) {
    Rz = (-z-25000.0)/150000.0;
  }
  double Omega = fmax(0.0, fmin(1.0, 1.0-Rz));
  
  params[2] = sigzz;
  params[0] = Omega*(bii[0]*(params[2]+Pf)-Pf)+(1.0-Omega)*params[2];
  params[1] = Omega*(bii[1]*(params[2]+Pf)-Pf)+(1.0-Omega)*params[2];
  params[3] = Omega*(bii[3]*(params[2]+Pf));
  params[4] = Omega*(bii[4]*(params[2]+Pf));
  params[5] = Omega*(bii[5]*(params[2]+Pf));
  params[0] += Pf;
  params[1] += Pf;
  params[2] += Pf;
}

void assertEqual(easi::Matrix<double> const& x, unsigned index, std::vector<std::vector<double>> const& material) {
  double params[6];
  model(x(index, 0), x(index, 1), x(index, 2), params);
  
  for (unsigned i = 0; i < 6; ++i) {
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
      {1, { 50000.0, 0.0, -1400.0}},
      {1, { 50000.0, 0.0, -50000.0}},
      {1, { 50000.0, 0.0, -49000.0}},
      {1, { 50000.0, 0.0, -3000.0}},
      {1, { 50000.0, 0.0, -22000.0}},
      {1, { 0.0, 100000.0, -1400.0}},
      {1, { 0.0, 100000.0, -50000.0}},
      {1, { 0.0, 100000.0, -49000.0}},
      {1, { 0.0, 100000.0, -3000.0}},
      {1, { 0.0, 100000.0, -22000.0}},
      {1, { 0.0, 0.0,  0.0}}
    });
  easi::Query queryRef = query;
  std::vector<std::string> parameters{"s_xx", "s_yy", "s_zz", "s_xy", "s_yz", "s_xz"};
  auto material = genericModel(argv[1], query, parameters);
  
  for (unsigned i = 0; i < 6; ++i) {
    assertEqual(query.x, i, material);
  }

  return 0;
}
