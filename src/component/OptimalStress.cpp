#include "easi/component/OptimalStress.h"

#include <cmath>

namespace easi {

// COMPUTE NORMALIZED STRESS FOLLOWING THE METHOD OF Ulrich et al. (2018)
void OptimalStress::evaluate() {
    double const pi = 3.141592653589793;
    // most favorable direction (A4, AM2003)
    double Phi = pi / 4.0 - 0.5 * atan(i.mu_s);
    double strike_rad = i.strike * pi / 180.0;
    double dip_rad = i.dip * pi / 180.0;
    double rake_rad = i.rake * pi / 180.0;
    double s2 = sin(2.0 * Phi);
    double c2 = cos(2.0 * Phi);
    double alpha = (2.0 * i.s2ratio - 1.0) / 3.0;
    double effectiveConfiningStress = std::fabs(i.effectiveConfiningStress);

    double ds = (i.mu_d * effectiveConfiningStress +
                 i.R * (i.cohesion + (i.mu_s - i.mu_d) * effectiveConfiningStress)) /
                (s2 + i.mu_d * (alpha + c2) + i.R * (i.mu_s - i.mu_d) * (alpha + c2));
    double sm = effectiveConfiningStress - alpha * ds;
    // sii are all positive
    double s11 = sm + ds;
    double s22 = sm - ds + 2.0 * ds * i.s2ratio;
    double s33 = sm - ds;

    double cd = cos(-Phi);
    double sd = sin(-Phi);

    double cr = cos(rake_rad);
    double sr = sin(rake_rad);

    double ci = cos(dip_rad);
    double si = sin(dip_rad);

    double cs = cos(strike_rad);
    double ss = sin(strike_rad);

    o.b_xx = -((ci * ci * s11 * sr * sr + s33 * si * si) * cd * cd +
               2 * ci * sd * si * sr * (s33 - s11) * cd +
               (s33 * sd * sd * sr * sr + cr * cr * s22) * ci * ci + sd * sd * si * si * s11) *
                 cs * cs +
             2 * cr * cs *
                 (cd * cd * ci * sr * s11 + sd * si * (s33 - s11) * cd -
                  ci * sr * (-s33 * sd * sd + s22)) *
                 ss -
             ss * ss * (cd * cd * cr * cr * s11 + cr * cr * s33 * sd * sd + s22 * sr * sr);
    o.b_xy = cr *
                 (cd * cd * ci * sr * s11 + sd * si * (s33 - s11) * cd -
                  ci * sr * (-s33 * sd * sd + s22)) *
                 cs * cs +
             ss *
                 ((-cd * cd * s11 + ci * ci * s22 - s33 * sd * sd) * cr * cr +
                  (ci * ci * s11 * sr * sr + s33 * si * si) * cd * cd +
                  2 * ci * sd * si * sr * (s33 - s11) * cd + s33 * ci * ci * sd * sd * sr * sr +
                  sd * sd * si * si * s11 - s22 * sr * sr) *
                 cs -
             ss * ss * cr *
                 (cd * cd * ci * sr * s11 + sd * si * (s33 - s11) * cd -
                  ci * sr * (-s33 * sd * sd + s22));
    o.b_xz = cd * cs * sd * sr * (s33 - s11) * si * si -
             ((cs * (-s11 * sr * sr + s33) * ci + cr * sr * ss * s11) * cd * cd -
              cs * ((s33 * sr * sr - s11) * sd * sd + cr * cr * s22) * ci -
              cr * sr * ss * (-s33 * sd * sd + s22)) *
                 si -
             cd * ci * sd * (ci * cs * sr - cr * ss) * (s33 - s11);
    o.b_yy = -((ci * ci * s11 * sr * sr + s33 * si * si) * cd * cd +
               2 * ci * sd * si * sr * (s33 - s11) * cd +
               (s33 * sd * sd * sr * sr + cr * cr * s22) * ci * ci + sd * sd * si * si * s11) *
                 ss * ss -
             2 * cr * cs *
                 (cd * cd * ci * sr * s11 + sd * si * (s33 - s11) * cd -
                  ci * sr * (-s33 * sd * sd + s22)) *
                 ss -
             cs * cs * (cd * cd * cr * cr * s11 + cr * cr * s33 * sd * sd + s22 * sr * sr);
    o.b_yz = -cd * sd * sr * ss * (s33 - s11) * si * si -
             ((-ss * (-s11 * sr * sr + s33) * ci + cr * cs * sr * s11) * cd * cd +
              ss * ((s33 * sr * sr - s11) * sd * sd + cr * cr * s22) * ci -
              cr * cs * sr * (-s33 * sd * sd + s22)) *
                 si +
             cd * ci * sd * (ci * sr * ss + cr * cs) * (s33 - s11);
    o.b_zz = -(cd * cd * s11 * sr * sr + s33 * sd * sd * sr * sr + cr * cr * s22) * si * si -
             2 * cd * ci * sd * sr * (s11 - s33) * si - ci * ci * (cd * cd * s33 + s11 * sd * sd);
}

} // namespace easi
