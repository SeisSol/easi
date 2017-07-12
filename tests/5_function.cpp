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

double rho(double y) {
  return 1600. + 59.5*pow(y, 1./3.);
}

double lambda(double y) {
  return pow(260. + 30*sqrt(y), 2.) * (1600. + 59.5*pow(y,1./3.));
}

int main(int argc, char** argv) {
  assert(argc == 2);

  easi::Query query = createQuery<3>({
      {7, { 0.0,   0.0, 0.0}},
      {5, { 0.0,  10.0, 0.0}},
      {4, { 0.0,  20.0, 0.0}},
      {3, { 0.0,  25.0, 0.0}}
    });
  
  auto material = elasticModel(argv[1], query);

  assert(equal(material[0].rho,      2500.0));
  assert(equal(material[0].mu,          0.0));
  assert(equal(material[0].lambda,  1.96e10));

  assert(equal(material[1].rho,       rho(10.0)));
  assert(equal(material[1].mu,              0.0));
  assert(equal(material[1].lambda, lambda(10.0)));

  assert(equal(material[2].rho,       rho(20.0)));
  assert(equal(material[2].mu,              0.0));
  assert(equal(material[2].lambda, lambda(20.0)));

  assert(equal(material[3].rho,       rho(25.0)));
  assert(equal(material[3].mu,              0.0));
  assert(equal(material[3].lambda, lambda(25.0)));

  return 0;
}
