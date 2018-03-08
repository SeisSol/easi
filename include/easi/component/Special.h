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
      double mu_d, mu_s, strike, dip, s_zz, cohesion, R, DipSlipFaulting, s2ratio;
    };
    in i;
    
    struct out {
      double b_xx, b_yy, b_zz, b_xy, b_yz, b_xz;
    };
    out o;
    
    inline void evaluate();
  };
}

SELF_AWARE_STRUCT(easi::STRESS_STR_DIP_SLIP_AM::in, mu_d, mu_s, strike, dip, s_zz, cohesion, R, DipSlipFaulting, s2ratio)
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
  double alpha = (2.0*i.s2ratio-1.0)/3.0;
  double effectiveConfiningStress = abs(i.s_zz);

  if (fabs(i.DipSlipFaulting) <= std::numeric_limits<double>::epsilon()) {
     //in case of Strike Slip faulting s_zz in actually the effective confining stress = sum(sii)/3 -Pf
     double ds = (i.mu_d * effectiveConfiningStress + i.R*(i.cohesion + (i.mu_s-i.mu_d)*effectiveConfiningStress)) / (s2 + i.mu_d*(alpha + c2) + i.R*(i.mu_s-i.mu_d)*(alpha + c2));
     double sm = effectiveConfiningStress - alpha*ds;
     //sii are all positive
     double s11 = sm + ds;
     double s22= sm - ds + 2.0*ds*i.s2ratio;
     double s33 = sm - ds;

     double cd = cos(Phi);
     double sd = sin(Phi);

     double ci = cos(dip_rad);
     double si = sin(dip_rad);

     double cs = cos(strike_rad);
     double ss = sin(strike_rad);

     //minus signs added to fall back to SeisSol convention (compressive stress<0)
     o.b_xx =  -(cd * cd * cs * cs * s33 * si * si + cs * cs * s11 * sd * sd * si * si + 2 * cd * cs * s11 * sd * si * ss - 2 * cd * cs * s33 * sd * si * ss + cd * cd * s11 * ss * ss + ci * ci * cs * cs * s22 + s33 * sd * sd * ss * ss);
     o.b_xy = -(-cd * cd * cs * s33 * si * si * ss - cs * s11 * sd * sd * si * si * ss + cd * cs * cs * s11 * sd * si - cd * cs * cs * s33 * sd * si - cd * s11 * sd * si * ss * ss + cd * s33 * sd * si * ss * ss + cd * cd * cs * s11 * ss - ci * ci * cs * s22 * ss + cs * s33 * sd * sd * ss);
     o.b_xz = -(cd * cd * ci * cs * s33 * si + ci * cs * s11 * sd * sd * si + cd * ci * s11 * sd * ss - cd * ci * s33 * sd * ss - cs * ci * s22 * si);
     o.b_yy = -(cd * cd * s33 * si * si * ss * ss + s11 * sd * sd * si * si * ss * ss - 2 * cd * cs * s11 * sd * si * ss + 2 * cd * cs * s33 * sd * si * ss + cd * cd * cs * cs * s11 + ci * ci * s22 * ss * ss + cs * cs * s33 * sd * sd);
     o.b_yz = -(-cd * cd * ci * s33 * si * ss - ci * s11 * sd * sd * si * ss + cd * ci * cs * s11 * sd - cd * ci * cs * s33 * sd + ss * ci * s22 * si);
     o.b_zz =  -(cd * cd * ci * ci * s33 + ci * ci * s11 * sd * sd + si * si * s22);

  } else {

     /* ds (delta_sigma) is deduced from R (A5, Aochi and Madariaga 2003),
      * assuming that sig1 and sig3 are in the yz plane
      * sigzz and sigma_ini are then related by a phi+dip rotation (using A3, AM03)
      * s_zz = sm  - ds * cos(2.*(Phi+dip_rad))
      * Now we have to assume that P = sm (not any more equal to s_zz)
      * and we can obtain the new expression of ds: */
     double ds = (i.mu_d * i.s_zz + i.R*(i.cohesion + (i.mu_s-i.mu_d)*i.s_zz)) / (s2 + i.mu_d*c2bis + i.R*(i.mu_s-i.mu_d)*c2bis);
     double sm = i.s_zz + ds * c2xyz;

     double s11 = sm + ds;
     double s22= sm - ds + 2.0*ds*i.s2ratio;
     double s33 = sm - ds;
     double cd = cos(phi_xyz);
     double sd = sin(phi_xyz);
     double cs = cos(strike_rad);
     double ss = sin(strike_rad);
 
     /* return a normalized stress tensor*/
     o.b_xx = szzInv * (cd * cd * cs * cs * s11 + cs * cs * s33 * sd * sd + ss * ss * s22);
     o.b_yy = szzInv * (cd * cd * s11 * ss * ss + s33 * sd * sd * ss * ss + cs * cs * s22);
     o.b_zz = szzInv * (cd * cd * s33 + sd * sd * s11);
     o.b_xy = szzInv * (-cd * cd * cs * s11 * ss - cs * s33 * sd * sd * ss + ss * s22 * cs);
     o.b_yz = szzInv * (-ss * cd * s11 * sd + ss * sd * s33 * cd);
     o.b_xz = szzInv * (cs * cd * s11 * sd - cs * sd * s33 * cd);
   }
  
}

#endif
