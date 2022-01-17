#include "easi/YAMLParser.h"
#include <algorithm>

int main(int argc, char** argv) {
    assert(argc == 2);

    easi::YAMLParser parser(3);
    easi::Component* model = parser.parse(argv[1]);

    std::set<std::string> supplied = model->suppliedParameters();
    std::set<std::string> expected = {"rho",  "mu",   "lambda", "plastCo", "bulkFriction", "s_xx",
                                      "s_yy", "s_zz", "s_xy",   "s_yz",    "s_xz"};

    std::set<std::string> missing;
    std::set_difference(expected.begin(), expected.end(), supplied.begin(), supplied.end(),
                        std::inserter(missing, missing.begin()));

    assert(missing.empty());

    delete model;

    return 0;
}
