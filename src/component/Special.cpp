#include "easi/component/Special.h"

#include <cmath>
#include <limits>

namespace easi {

// COMPUTE NORMALIZED STRESS FOLLOWING THE METHOD  OF Aochi and Madariaga 2004 extended to dip slip
// fault This module is deprecated: use OptimalStress instead We nevertheless keep it to be able to
// run the exact Sumatra SC simulation with the lastest SeisSol
void STRESS_STR_DIP_SLIP_AM::evaluate() {
    double const pi = 3.141592653589793;
    // most favorable direction (A4, AM2003)
    double Phi = pi / 4.0 - 0.5 * atan(i.mu_s);
    double strike_rad = i.strike * pi / 180.0;
    double dip_rad = i.dip * pi / 180.0;
    double phi_xyz = Phi - dip_rad;
    double s2 = sin(2.0 * Phi);
    double c2 = cos(2.0 * Phi);
    double c2xyz = cos(2.0 * phi_xyz);
    double c2bis = c2 - c2xyz;
    double szzInv = 1.0 / i.s_zz;

    if (fabs(i.DipSlipFaulting) <= std::numeric_limits<double>::epsilon()) {
        double ds = (i.mu_d * i.s_zz + i.R * (i.cohesion + (i.mu_s - i.mu_d) * i.s_zz)) /
                    (s2 + i.mu_d * c2 + i.R * (i.mu_s - i.mu_d) * c2);
        double sm = i.s_zz;
        double s11 = sm + ds;
        double s22 = sm - ds + 2.0 * ds * i.s2ratio;
        double s33 = sm - ds;

        double cd = cos(Phi);
        double sd = sin(Phi);

        double ci = cos(dip_rad);
        double si = sin(dip_rad);

        double cs = cos(strike_rad);
        double ss = sin(strike_rad);

        o.b_xx =
            szzInv * (cd * cd * cs * cs * s33 * si * si + cs * cs * s11 * sd * sd * si * si +
                      2 * cd * cs * s11 * sd * si * ss - 2 * cd * cs * s33 * sd * si * ss +
                      cd * cd * s11 * ss * ss + ci * ci * cs * cs * s22 + s33 * sd * sd * ss * ss);
        o.b_xy =
            szzInv * (-cd * cd * cs * s33 * si * si * ss - cs * s11 * sd * sd * si * si * ss +
                      cd * cs * cs * s11 * sd * si - cd * cs * cs * s33 * sd * si -
                      cd * s11 * sd * si * ss * ss + cd * s33 * sd * si * ss * ss +
                      cd * cd * cs * s11 * ss - ci * ci * cs * s22 * ss + cs * s33 * sd * sd * ss);
        o.b_xz = szzInv * (cd * cd * ci * cs * s33 * si + ci * cs * s11 * sd * sd * si +
                           cd * ci * s11 * sd * ss - cd * ci * s33 * sd * ss - cs * ci * s22 * si);
        o.b_yy =
            szzInv * (cd * cd * s33 * si * si * ss * ss + s11 * sd * sd * si * si * ss * ss -
                      2 * cd * cs * s11 * sd * si * ss + 2 * cd * cs * s33 * sd * si * ss +
                      cd * cd * cs * cs * s11 + ci * ci * s22 * ss * ss + cs * cs * s33 * sd * sd);
        o.b_yz = szzInv * (-cd * cd * ci * s33 * si * ss - ci * s11 * sd * sd * si * ss +
                           cd * ci * cs * s11 * sd - cd * ci * cs * s33 * sd + ss * ci * s22 * si);
        o.b_zz = szzInv * (cd * cd * ci * ci * s33 + ci * ci * s11 * sd * sd + si * si * s22);

    } else {

        /* ds (delta_sigma) is deduced from R (A5, Aochi and Madariaga 2003),
         * assuming that sig1 and sig3 are in the yz plane
         * sigzz and sigma_ini are then related by a phi+dip rotation (using A3, AM03)
         * s_zz = sm  - ds * cos(2.*(Phi+dip_rad))
         * Now we have to assume that P = sm (not any more equal to s_zz)
         * and we can obtain the new expression of ds: */
        double ds = (i.mu_d * i.s_zz + i.R * (i.cohesion + (i.mu_s - i.mu_d) * i.s_zz)) /
                    (s2 + i.mu_d * c2bis + i.R * (i.mu_s - i.mu_d) * c2bis);
        double sm = i.s_zz + ds * c2xyz;

        double s11 = sm + ds;
        double s22 = sm - ds + 2.0 * ds * i.s2ratio;
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
}

} // namespace easi
