#include "easitest.h"
#include <mpi.h>

int main(int argc, char** argv) {
    assert(argc == 2);

    MPI_Init(&argc, &argv);

    easi::Query query = createQuery<3>({{1, {2.0, -1.0, 0.0}},
                                        {1, {3.0, 0.0, 0.0}},
                                        {1, {2.5, -0.5, 0.0}},
                                        {1, {2.75, -0.25, 0.0}},
                                        {1, {2.6, -0.25, 0.0}}});
    auto material = elasticModel(argv[1], query);

    assert(equal(material[0].rho, 1.0));
    assert(equal(material[0].mu, -1.0));
    assert(equal(material[0].lambda, 10.0));

    assert(equal(material[1].rho, 6.0));
    assert(equal(material[1].mu, -6.0));
    assert(equal(material[1].lambda, 15.0));

    assert(equal(material[2].rho, 5.0));
    assert(equal(material[2].mu, -5.0));
    assert(equal(material[2].lambda, 14.0));

    assert(equal(material[3].rho, 6.0));
    assert(equal(material[3].mu, -6.0));
    assert(equal(material[3].lambda, 15.0));

    assert(equal(material[4].rho, 5.0));
    assert(equal(material[4].mu, -5.0));
    assert(equal(material[4].lambda, 14.0));

    MPI_Finalize();

    return 0;
}
