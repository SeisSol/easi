#include "easitest.h"
#include <cmath>

double rho(double y) { return 1600. + 59.5 * pow(y, 1. / 3.); }

double lambda(double y) { return pow(260. + 30 * sqrt(y), 2.) * (1600. + 59.5 * pow(y, 1. / 3.)); }

int main(int argc, char** argv) {
    assert(argc == 2);

    easi::Query query = createQuery<3>({{7, {0.0, 0.0, 0.0}},
                                        {5, {0.0, 10.0, 0.0}},
                                        {4, {0.0, 20.0, 0.0}},
                                        {3, {0.0, 25.0, 0.0}}});

    auto material = elasticModel(argv[1], query);

    assert(equal(material[0].rho, 2500.0));
    assert(equal(material[0].mu, 0.0));
    assert(equal(material[0].lambda, 1.96e10));

    assert(equal(material[1].rho, rho(10.0)));
    assert(equal(material[1].mu, 0.0));
    assert(equal(material[1].lambda, lambda(10.0)));

    assert(equal(material[2].rho, rho(20.0)));
    assert(equal(material[2].mu, 0.0));
    assert(equal(material[2].lambda, lambda(20.0)));

    assert(equal(material[3].rho, rho(25.0)));
    assert(equal(material[3].mu, 0.0));
    assert(equal(material[3].lambda, lambda(25.0)));

    return 0;
}
