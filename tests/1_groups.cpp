#include "easitest.h"

int main(int argc, char** argv) {
    assert(argc == 2);

    easi::Query query =
        createQuery<3>({{1, {1.0, 2.0, 3.0}}, {2, {-3.0, -2.0, -1.0}}, {1, {-1.0, -2.0, -3.0}}});

    auto material = elasticModel(argv[1], query);

    assert(equal(material[0].rho, 2600.0));
    assert(equal(material[0].mu, 1.04e10));
    assert(equal(material[0].lambda, 2.08e10));

    assert(equal(material[1].rho, 2700.0));
    assert(equal(material[1].mu, 3.23980992e10));
    assert(equal(material[1].lambda, 3.24038016e10));

    assert(equal(material[2].rho, 2600.0));
    assert(equal(material[2].mu, 1.04e10));
    assert(equal(material[2].lambda, 2.08e10));

    return 0;
}
