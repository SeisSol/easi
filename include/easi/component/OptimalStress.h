#ifndef EASI_COMPONENT_OPTIMALSTRESS_H_
#define EASI_COMPONENT_OPTIMALSTRESS_H_

#include "easi/util/Magic.h"
#include "easi/util/MagicStruct.h"

#include <string>

namespace easi {

struct OptimalStress {
    struct in {
        double mu_d, mu_s, strike, dip, rake, effectiveConfiningStress, cohesion, R, s2ratio;
    };
    in i;

    struct out {
        double b_xx, b_yy, b_zz, b_xy, b_yz, b_xz;
    };
    out o;

    void evaluate();
};

} // namespace easi

SELF_AWARE_STRUCT(easi::OptimalStress::in, mu_d, mu_s, strike, dip, rake, effectiveConfiningStress,
                  cohesion, R, s2ratio)
SELF_AWARE_STRUCT(easi::OptimalStress::out, b_xx, b_yy, b_zz, b_xy, b_yz, b_xz)

#endif
