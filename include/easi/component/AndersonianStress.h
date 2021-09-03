#ifndef EASI_COMPONENT_ANDERSONIANSTRESS_H_
#define EASI_COMPONENT_ANDERSONIANSTRESS_H_

#include "easi/util/Magic.h"
#include "easi/util/MagicStruct.h"

#include <string>

namespace easi {

struct AndersonianStress {
    struct in {
        double mu_d, mu_s, SH_max, sig_zz, cohesion, S, s2ratio, S_v;
    };
    in i;

    struct out {
        double b_xx, b_yy, b_zz, b_xy, b_yz, b_xz;
    };
    out o;

    void evaluate();
};

} // namespace easi

SELF_AWARE_STRUCT(easi::AndersonianStress::in, mu_d, mu_s, SH_max, sig_zz, cohesion, S, s2ratio,
                  S_v)
SELF_AWARE_STRUCT(easi::AndersonianStress::out, b_xx, b_yy, b_zz, b_xy, b_yz, b_xz)

#endif
