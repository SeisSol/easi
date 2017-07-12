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

/* Uses error propagation rules from http://lectureonline.cl.msu.edu/~mmp/labs/error/e2.htm
* rho_SI = 1e3 * rho; drho_SI = 1e3 * drho
* mu = 1e9 * rho * vs^2; dmu = 1e9 * vs * sqrt[(vs*drho)^2 + (2*rho*dvs)^2]
* lambda = 1e9 * rho * (vp^2 - 2*vs^2); dlambda = 1e9 * sqrt[((vp^2 - 2*vs^2)*drho)^2 + (2*rho*vs*dvs)^2 + (4*rho*vp*dvp)^2]
*/
class Converter {
public:
  Converter(double rho, double vp, double vs, double d) : m_rho(rho), m_vp(vp), m_vs(vs), m_d(d) {}
  
  double rho()    const { return 1.0e3 * m_rho; }
  double mu()     const { return 1.0e9 * m_rho * pow(m_vs, 2.); }
  double lambda() const { return 1.0e9 * m_rho * (pow(m_vp, 2.) - 2.*pow(m_vs, 2.)); }
  
  double drho()    const { return 1.0e3 * m_d; }
  double dmu()     const { return 1.0e9 * sqrt(pow(pow(m_vs, 2.) * m_d, 2.) + pow(2.0 * m_rho * m_vs * m_d, 2.)); }
  double dlambda() const { return 1.0e9 * sqrt(pow((pow(m_vp, 2.) - 2.*pow(m_vs, 2.)) * m_d, 2.) + pow(2.0 * m_rho * m_vp * m_d, 2.) + pow(-2.0 * 2.0 * m_rho * m_vs * m_d, 2.)); }

private:
  double m_rho, m_vp, m_vs, m_d;
};

void assertEqual(ElasticMaterial const& material, Converter const& converter) {
  assert(equal(material.rho,       converter.rho(),    converter.drho()));
  assert(equal(material.mu,         converter.mu(),     converter.dmu()));
  assert(equal(material.lambda, converter.lambda(), converter.dlambda()));
}

int main(int argc, char** argv) {
  assert(argc == 2);

  easi::Query query = createQuery<3>({
      {1, { 1000000.0,  0.0,  0.0}}, // Inner core
      {1, { 2000000.0,  0.0,  0.0}}, // Outer core
      {1, { 3500000.0,  0.0,  0.0}}, // Lower mantle (1)
      {1, { 4000000.0,  0.0,  0.0}}, // Lower mantle (2)
      {1, { 5650000.0,  0.0,  0.0}}, // Lower mantle (3)
      {1, { 5736000.0,  0.0,  0.0}}, // Transition zone (1)
      {1, { 5821000.0,  0.0,  0.0}}, // Transition zone (2)
      {1, { 6016000.0,  0.0,  0.0}}, // Transition zone (3)
      {1, { 6356000.0,  0.0,  0.0}}, // Crust (1)
      {1, { 6368000.0,  0.0,  0.0}}, // Crust (2)
      {1, { 6371000.0,  0.0,  0.0}}  // Ocean
    });
  
  auto material = elasticModel(argv[1], query);

  // Assertions are based on tables given in Dziewonski and Anderson paper.
  Converter innerCore();
  assertEqual( material[0], Converter(12.87073, 11.10542, 3.55823, 1e-4)); // r=1000000
  assertEqual( material[1], Converter(11.65478,  9.83496,     0.0, 1e-4)); // r=2000000
  assertEqual( material[2], Converter( 5.55641, 13.71168, 7.26486, 1e-4)); // r=3500000
  assertEqual( material[3], Converter( 5.30724, 13.24532, 7.09974, 1e-4)); // r=4000000
  assertEqual( material[4], Converter( 4.41241, 10.91005, 6.09418, 1e-4)); // r=5650000
  assertEqual( material[5], Converter( 3.98399, 10.21203, 5.54311, 1e-4)); // r=5736000
  assertEqual( material[6], Converter( 3.91282,  9.90185, 5.37014, 1e-4)); // r=5821000
  assertEqual( material[7], Converter( 3.51639,  8.81867, 4.73840, 1e-4)); // r=6016000
  assertEqual( material[8], Converter( 2.90000,  6.80000, 3.90000, 1e-4)); // r=6356000
  assertEqual( material[9], Converter( 2.60000,  5.80000, 3.20000, 1e-4)); // r=6368000
  assertEqual(material[10], Converter( 1.02000,  1.45000,     0.0, 1e-4)); // r=6371000

  return 0;
}
