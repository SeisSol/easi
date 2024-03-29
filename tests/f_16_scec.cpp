#include "easitest.h"
#include <cmath>

void model(double x, double z, double params[9]) {
    params[0] = 0.0;           // s_xx
    params[2] = 0.0;           // s_zz
    params[5] = 0.0;           // s_xz
    params[4] = 0.0;           // s_yz
    params[1] = -6.000000E+07; // s_yy
    params[6] = 6.770000E-01;  // mu_s
    params[7] = 3.730000E-01;  // mu_d
    params[8] = 4.000000E-01;  // d_c
    params[10] = 1.000000E+09; // forced_rupture_time

    z *= -1.0;

    if (x <= 0.0 && z <= 0.0) {
        params[3] = 2.509795E+07;  // s_xy
        params[9] = -4.000000E+06; // cohesion
    } else if (x <= 75.0 && z <= 0.0) {
        double xi = x / 75.0;
        params[3] = 2.509795E+07 * (1.0 - xi) + 2.523141E+07 * xi; // s_xy
        params[9] = -4.000000E+06;                                 // cohesion
    } else if (x >= 75.0 && x <= 150.0 && z >= 150.0 && z <= 225.0) {
        double xi = (x - 75.0) / 75.0;
        double eta = (z - 150.0) / 75.0;
        params[3] = (2.444593E+07 * (1.0 - eta) + 2.445351E+07 * eta) * (1.0 - xi) +
                    (2.488395E+07 * (1.0 - eta) + 2.498051E+07 * eta) * xi; // s_xy
        params[9] = -((3.800000E+06 * (1.0 - eta) + 3.700000E+06 * eta) * (1.0 - xi) +
                      (3.800000E+06 * (1.0 - eta) + 3.700000E+06 * eta) * xi); // cohesion
    } else {
        params[3] = 2.498051E+07;  // s_xy
        params[9] = -3.700000E+06; // cohesion
    }
}

void assertEqual(double x, double z, unsigned index,
                 std::vector<std::vector<double>> const& material) {
    double params[11];
    model(x, z, params);

    for (unsigned i = 0; i < 11; ++i) {
        assert(equal(material[i][index], params[i],
                     fabs(std::numeric_limits<double>::epsilon() * params[i])));
    }
}

int main(int argc, char** argv) {
    assert(argc == 2);

    easi::Query query = createQuery<3>({{1, {-5.0, 0.0, 5.0}},
                                        {1, {25.0, 0.0, 5.0}},
                                        {1, {100.0, 0.0, -200.0}},
                                        {1, {1000.0, 0.0, -1000.0}}});
    std::vector<std::string> parameters{"s_xx",
                                        "s_yy",
                                        "s_zz",
                                        "s_xy",
                                        "s_yz",
                                        "s_xz",
                                        "mu_s",
                                        "mu_d",
                                        "d_c",
                                        "cohesion",
                                        "forced_rupture_time"};
    auto material = genericModel(argv[1], query, parameters);

    assertEqual(-5.0, 5.0, 0, material);
    assertEqual(25.0, 5.0, 1, material);
    assertEqual(100.0, -200.0, 2, material);
    assertEqual(1000.0, -1000.0, 3, material);

    return 0;
}
