#include "easitest.h"

int main(int argc, char** argv) {
    assert(argc == 2);

    easi::Query query = createQuery<3>({
        {1, {0.0, 0.0, 0.0}},
        {1, {0.0, 0.0, -2000.0}},
        {1, {0.0, 0.0, -2100.0}},
        {1, {0.0, 0.0, -2500.0}},
        {1, {0.0, 0.0, -50000.0}},
    });

    auto material = elasticModel(argv[1], query);

    assert(equal(material[0].rho, 2600.0));
    assert(equal(material[0].mu, 1.04e10));
    assert(equal(material[0].lambda, 2.08e10));

    assert(equal(material[1].rho, 2700.0));
    assert(equal(material[1].mu, 3.24e10));
    assert(equal(material[1].lambda, 3.24e10));

    assert(equal(material[2].rho, 2730.0));
    assert(equal(material[2].mu, 3.396e10));
    assert(equal(material[2].lambda, 3.036e10));

    assert(equal(material[3].rho, 2850.0));
    assert(equal(material[3].mu, 4.02e10));
    assert(equal(material[3].lambda, 2.22e10));

    assert(equal(material[4].rho, 3000.0));
    assert(equal(material[4].mu, 4.80e10));
    assert(equal(material[4].lambda, 1.20e10));

    return 0;
}
