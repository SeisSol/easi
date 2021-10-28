#include "easitest.h"

void model(double y, double params[3]) {
    params[0] = 2670.0;
    if (y <= -800.0) {
        params[1] = 2.816717568E+10;
        params[2] = 2.817615756E+10;
    } else if (y >= -800.0 && y <= 800.0) {
        params[1] = 1.251489075E+10;
        params[2] = 1.251709350E+10;
    } else {
        params[1] = 3.203812032E+10;
        params[2] = 3.204375936E+10;
    }
}

void assertEqual(double y, ElasticMaterial const& material) {
    double params[3];
    model(y, params);

    assert(equal(material.rho, params[0]));
    assert(equal(material.mu, params[1]));
    assert(equal(material.lambda, params[2]));
}

int main(int argc, char** argv) {
    assert(argc == 2);

    easi::Query query = createQuery<3>({{1, {0.0, -1000.0, 0.0}},
                                        {1, {0.0, -800.0, 0.0}},
                                        {1, {0.0, 0.0, 0.0}},
                                        {1, {0.0, 800.0, 0.0}},
                                        {1, {0.0, 1000.0, 0.0}}});

    auto material = elasticModel(argv[1], query);

    assertEqual(-1000.0, material[0]);
    assertEqual(-800.0, material[1]);
    assertEqual(0.0, material[2]);
    assertEqual(800.0, material[3]);
    assertEqual(1000.0, material[4]);

    return 0;
}
