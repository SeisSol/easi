#ifdef EASI_USE_IMPALAJIT
#include "easi/component/FunctionMap.h"

#include <impalajit.hh>

#include <cassert>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <utility>

namespace easi {

Matrix<double> FunctionMap::map(Matrix<double>& x) {
    assert(x.cols() == dimDomain());
    assert(m_functions.size() == dimCodomain());

    Matrix<double> y(x.rows(), dimCodomain());
    for (unsigned i = 0; i < y.rows(); ++i) {
        for (unsigned j = 0; j < y.cols(); ++j) {
            y(i, j) = m_functionWrapper(m_functions[j], x, i);
        }
    }
    return y;
}

void FunctionMap::setMap(std::set<std::string> const& in, OutMap const& functionMap) {
    m_functions.clear();
    std::vector<std::string> functionDefinitions;

    auto inIt = in.cbegin();
    std::ostringstream ss;
    ss << '(' << *inIt++;
    while (inIt != in.cend()) {
        ss << ',' << *inIt++;
    }
    ss << ") {";
    for (auto it = functionMap.cbegin(); it != functionMap.cend(); ++it) {
        functionDefinitions.push_back(it->first + ss.str() + it->second + "}");
    }
    impalajit::Compiler compiler(functionDefinitions);
    compiler.compile();

    unsigned dimDomain = std::numeric_limits<unsigned>::max();
    for (auto it = functionMap.begin(); it != functionMap.end(); ++it) {
        m_functions.push_back(compiler.getFunction(it->first));
        unsigned funDomain = compiler.getParameterCount(it->first);
        if (dimDomain != funDomain && dimDomain != std::numeric_limits<unsigned>::max()) {
            throw std::invalid_argument(
                addFileReference("All functions in a FunctionMap must have the same domain."));
        }
        dimDomain = funDomain;
    }

    std::set<std::string> out;
    for (auto const& kv : functionMap) {
        out.insert(kv.first);
    }

    setIn(in);
    setOut(out);

    m_functionWrapper = getFunctionWrapper(this->dimDomain());
}

} // namespace easi
#endif // EASI_USE_IMPALAJIT
