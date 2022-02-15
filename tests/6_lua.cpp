#include "easitest.h"
#include <cmath>
#include <iostream>

double effectiveConfiningStress(double z) {
    const double elevation = 100;
    return 2670.0*0.21*9.8*std::min(-100.0,elevation-z);
}

int main(int argc, char** argv) {
    assert(argc == 2);

    easi::Query query = createQuery<3>({
        {0, {0.0, 0.0, 200.0}},
        {0, {0.0, 0.0, 210.0}},
        {0, {0.0, 0.0, 220.0}},
        {0, {0.0, 0.0, 230.0}}
    });
    std::vector<std::string> parameters{"effectiveConfiningStress"};
    auto stress = genericModel(argv[1], query, parameters);

    double curVal = 200.0;
    for (auto& s : stress[0]) {
        constexpr double eps = 10e-12;
        assert(std::abs(s - effectiveConfiningStress(curVal)) <= eps);
        curVal += 10.0;
    }



    return 0;
}

