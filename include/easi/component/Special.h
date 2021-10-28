#ifndef EASI_COMPONENT_SPECIAL_H_
#define EASI_COMPONENT_SPECIAL_H_

#include "easi/util/Magic.h"
#include "easi/util/MagicStruct.h"

#include <string>

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

    void evaluate();
};
} // namespace easi

SELF_AWARE_STRUCT(easi::STRESS_STR_DIP_SLIP_AM::in, mu_d, mu_s, strike, dip, s_zz, cohesion, R,
                  DipSlipFaulting, s2ratio)
SELF_AWARE_STRUCT(easi::STRESS_STR_DIP_SLIP_AM::out, b_xx, b_yy, b_zz, b_xy, b_yz, b_xz)

#endif
