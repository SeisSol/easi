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
#ifndef EASI_COMPONENT_SPECIAL_H_
#define EASI_COMPONENT_SPECIAL_H_

#include <cmath>
#include "easi/util/MagicStruct.h"

namespace easi {
  struct STRESS_STR_DIP_SLIP_AM {
    struct in {
      double mu_d, mu_s, strike, dip, s_zz, cohesion, R;
    };
    in i;
    
    struct out {
      double b_xx, b_yy, b_zz, b_xy, b_yz, b_xz;
    };
    out o;
    
    void evaluate();
  };
}

SELF_AWARE_STRUCT(easi::STRESS_STR_DIP_SLIP_AM::in, mu_d, mu_s, strike, dip, s_zz, cohesion, R)
SELF_AWARE_STRUCT(easi::STRESS_STR_DIP_SLIP_AM::out, b_xx, b_yy, b_zz, b_xy, b_yz, b_xz)

// COMPUTE NORMALIZED STRESS FOLLOWING THE METHOD  OF Aochi and Madariaga 2004 extended to dip slip fault
void easi::STRESS_STR_DIP_SLIP_AM::evaluate() {
  double const pi = 3.141592653589793;
  // most favorable direction (A4, AM2003)
  double Phi = pi/4.0-0.5*atan(i.mu_s);
  double strike_rad = i.strike*pi/180.0;
  double dip_rad = i.dip*pi/180.0;
  double phi_xyz = Phi-dip_rad;
  double s2 = sin(2.0*Phi);
  double c2 = cos(2.0*Phi);
  double c2xyz = cos(2.0*phi_xyz);
  double c2bis = c2 - c2xyz;  
  double szzInv = 1.0 / i.s_zz;

  /* ds (delta_sigma) is deduced from R (A5, Aochi and Madariaga 2003),
   * assuming that sig1 and sig3 are in the yz plane
   * sigzz and sigma_ini are then related by a phi+dip rotation (using A3, AM03)
   * s_zz = sm  - ds * cos(2.*(Phi+dip_rad))
   * Now we have to assume that P = sm (not any more equal to s_zz)
   * and we can obtain the new expression of ds: */
  double ds = (i.mu_d * i.s_zz + i.R*(i.cohesion + (i.mu_s-i.mu_d)*i.s_zz)) / (s2 + i.mu_d*c2bis + i.R*(i.mu_s-i.mu_d)*c2bis);
  double sm = i.s_zz + ds * c2xyz;

  double s11 = sm + ds;
  double s22 = sm;
  double s33 = sm - ds;
  
  double cd = cos(phi_xyz);
  double sd = sin(phi_xyz);
  double cs = cos(strike_rad);
  double ss = sin(strike_rad);
  
  o.b_xx = szzInv * (cd * cd * cs * cs * s11 + cs * cs * s33 * sd * sd + ss * ss * s22);
  o.b_yy = szzInv * (cd * cd * s11 * ss * ss + s33 * sd * sd * ss * ss + cs * cs * s22);
  o.b_zz = szzInv * (cd * cd * s33 + sd * sd * s11);
  o.b_xy = szzInv * (-cd * cd * cs * s11 * ss - cs * s33 * sd * sd * ss + ss * s22 * cs);
  o.b_yz = szzInv * (-ss * cd * s11 * sd + ss * sd * s33 * cd);
  o.b_xz = szzInv * (cs * cd * s11 * sd - cs * sd * s33 * cd);
}

#endif
