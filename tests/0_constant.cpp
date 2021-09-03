#include "easitest.h"

int main(int argc, char** argv) {
    assert(argc == 2);

    easi::Query query = createQuery<3>({{1, {1.0, 2.0, 3.0}}, {2, {-3.0, -2.0, -1.0}}});

    auto material = elasticModel(argv[1], query);

    assert(material[0].lambda == 1.0);
    assert(material[0].mu == 2.0);
    assert(material[0].rho == 3.0);

    assert(material[1].lambda == 1.0);
    assert(material[1].mu == 2.0);
    assert(material[1].rho == 3.0);

    return 0;
}
