/**
 * @file
 * This file is part of SeisSol.
 *
 * @author Thomas Ulrich (ulrich AT geophysik.uni-muenchen.de)
 *
 * @section LICENSE
 * Copyright (c) 2018, SeisSol Group
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
#ifndef EASI_COMPONENT_OPTIMALSTRESS_SZZ_H_
#define EASI_COMPONENT_OPTIMALSTRESS_SZZ_H_

#include <cmath>
#include "easi/util/MagicStruct.h"

namespace easi {
  struct OptimalStress_Szz {
    struct in {
      double mu_d, mu_s, strike, dip, rake, s_zz, cohesion, R0, s2ratio;
    };
    in i;
    
    struct out {
      double b_xx, b_yy, b_zz, b_xy, b_yz, b_xz;
    };
    out o;
    
    inline void evaluate();
  };
}

SELF_AWARE_STRUCT(easi::OptimalStress_Szz::in, mu_d, mu_s, strike, dip, rake, s_zz, cohesion, R0, s2ratio)
SELF_AWARE_STRUCT(easi::OptimalStress_Szz::out, b_xx, b_yy, b_zz, b_xy, b_yz, b_xz)

// COMPUTE NORMALIZED STRESS FOLLOWING THE METHOD OF Ulrich et al. (2018)
void easi::OptimalStress_Szz::evaluate() {
  double const pi = 3.141592653589793;
  // most favorable direction (A4, AM2003)
  double Phi = pi/4.0-0.5*atan(i.mu_s);
  double strike_rad = i.strike*pi/180.0;
  double dip_rad = i.dip*pi/180.0;
  double rake_rad = i.rake*pi/180.0;
  double s2 = sin(2.0*Phi);
  double c2 = cos(2.0*Phi);
  double alpha = (2.0*i.s2ratio-1.0)/3.0;

  double cd = cos(-Phi);
  double sd = sin(-Phi);

  double cr = cos(rake_rad);
  double sr = sin(rake_rad);

  double ci = cos(dip_rad);
  double si = sin(dip_rad);

  double cs = cos(strike_rad);
  double ss = sin(strike_rad);

  // a1, a2 and a3 are the coefficient of o.b_zz = -a1 s11  -a2 s22  -a3 s33;
  double a1 = pow(si * sr * cd + ci * sd, 2.0);
  double a2 = si * si * cr * cr;
  double a3 = pow(-si * sr * sd + ci * cd, 2.0);
  double a = a1 + a2 + a3;
  double b = a1 + (2.0*i.s2ratio-1.0)*a2 - a3;
  double mufac = (i.mu_d + i.R0 * (i.mu_s-i.mu_d));
  double ds = (std::fabs(i.s_zz) + i.R0 * a * std::fabs(i.cohesion) / mufac) / (b + a * c2 + a *s2 / mufac);
  double sm  = (std::fabs(i.s_zz) - b * ds) / a;

  //sii are all positive
  double s11 = sm + ds;
  double s22= sm - ds + 2.0*ds*i.s2ratio;
  double s33 = sm - ds;


  o.b_xx = -((ci * ci * s11 * sr * sr + s33 * si * si) * cd * cd + 2 * ci * sd * si * sr * (s33 - s11) * cd + (s33 * sd * sd * sr * sr + cr * cr * s22) * ci * ci + sd * sd * si * si * s11) * cs * cs + 2 * cr * cs * (cd * cd * ci * sr * s11 + sd * si * (s33 - s11) * cd - ci * sr * (-s33 * sd * sd + s22)) * ss - ss * ss * (cd * cd * cr * cr * s11 + cr * cr * s33 * sd * sd + s22 * sr * sr);
  o.b_xy = cr * (cd * cd * ci * sr * s11 + sd * si * (s33 - s11) * cd - ci * sr * (-s33 * sd * sd + s22)) * cs * cs + ss * ((-cd * cd * s11 + ci * ci * s22 - s33 * sd * sd) * cr * cr + (ci * ci * s11 * sr * sr + s33 * si * si) * cd * cd + 2 * ci * sd * si * sr * (s33 - s11) * cd + s33 * ci * ci * sd * sd * sr * sr + sd * sd * si * si * s11 - s22 * sr * sr) * cs - ss * ss * cr * (cd * cd * ci * sr * s11 + sd * si * (s33 - s11) * cd - ci * sr * (-s33 * sd * sd + s22));
  o.b_xz = cd * cs * sd * sr * (s33 - s11) * si * si - ((cs * (-s11 * sr * sr + s33) * ci + cr * sr * ss * s11) * cd * cd - cs * ((s33 * sr * sr - s11) * sd * sd + cr * cr * s22) * ci - cr * sr * ss * (-s33 * sd * sd + s22)) * si - cd * ci * sd * (ci * cs * sr - cr * ss) * (s33 - s11);
  o.b_yy = -((ci * ci * s11 * sr * sr + s33 * si * si) * cd * cd + 2 * ci * sd * si * sr * (s33 - s11) * cd + (s33 * sd * sd * sr * sr + cr * cr * s22) * ci * ci + sd * sd * si * si * s11) * ss * ss - 2 * cr * cs * (cd * cd * ci * sr * s11 + sd * si * (s33 - s11) * cd - ci * sr * (-s33 * sd * sd + s22)) * ss - cs * cs * (cd * cd * cr * cr * s11 + cr * cr * s33 * sd * sd + s22 * sr * sr);
  o.b_yz = -cd * sd * sr * ss * (s33 - s11) * si * si - ((-ss * (-s11 * sr * sr + s33) * ci + cr * cs * sr * s11) * cd * cd + ss * ((s33 * sr * sr - s11) * sd * sd + cr * cr * s22) * ci - cr * cs * sr * (-s33 * sd * sd + s22)) * si + cd * ci * sd * (ci * sr * ss + cr * cs) * (s33 - s11);
  o.b_zz = -(cd * cd * s11 * sr * sr + s33 * sd * sd * sr * sr + cr * cr * s22) * si * si - 2 * cd * ci * sd * sr * (s11 - s33) * si - ci * ci * (cd * cd * s33 + s11 * sd * sd);
  
}

#endif
