#include "easi/component/LayeredModelBuilder.h"
#include "easi/component/DomainFilter.h"
#include "easi/component/Map.h"
#include "easi/component/PolynomialMap.h"
#include "easi/util/Print.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace easi {

class Component;

void LayeredModelBuilder::setInterpolationType(std::string const& interpolationType) {
    std::string iType = interpolationType;
    std::transform(iType.begin(), iType.end(), iType.begin(), ::tolower);
    if (iType == "lower") {
        setInterpolationType(Lower);
    } else if (iType == "upper") {
        setInterpolationType(Upper);
    } else if (iType == "linear") {
        setInterpolationType(Linear);
    } else {
        std::stringstream ss;
        ss << "Invalid interpolation type " << interpolationType << ".";
        throw std::invalid_argument(ss.str());
    }
}

void LayeredModelBuilder::setNodes(Nodes const& nodes) {
    auto nParams = nodes.begin()->second.size();
    for (Nodes::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
        if (it->second.size() != nParams) {
            throw std::invalid_argument("The number of parameters must be equal for every node.");
        }
    }
    m_nodes = nodes;
}

Component* LayeredModelBuilder::getResult() {
    for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it) {
        if (std::isnan(it->first)) {
            throw std::invalid_argument("Not a number nodes are forbidden.");
        } else if (m_interpolationType == Linear && std::isinf(it->first)) {
            throw std::invalid_argument("Infinite nodes are forbidden for interpolation = linear.");
        }
    }
    Nodes::iterator lower = m_nodes.end();
    Nodes::iterator upper;
    for (upper = m_nodes.begin(); upper != m_nodes.end(); ++upper) {
        m_map->add(createModel(lower, upper, m_map->out()));
        lower = upper;
    }

    m_map->add(createModel(lower, upper, m_map->out()));

    return m_map;
}

Component* LayeredModelBuilder::createModel(Nodes::iterator& lower, Nodes::iterator& upper,
                                            std::set<std::string> const& in) {
    if (in.size() != 1) {
        std::ostringstream os;
        os << "Layered model requires 1D input (got ";
        printWithSeparator(in, os);
        os << ").";
        throw std::invalid_argument(os.str());
    }

    enum InterpolationType interpolationType;
    if (lower == m_nodes.end()) {
        interpolationType = Upper;
    } else if (upper == m_nodes.end()) {
        interpolationType = Lower;
    } else {
        interpolationType = m_interpolationType;
    }
    double lowerLimit =
        (lower != m_nodes.end()) ? lower->first : -std::numeric_limits<double>::infinity();
    double upperLimit =
        (upper != m_nodes.end()) ? upper->first : std::numeric_limits<double>::infinity();
    auto nParams = (lower != m_nodes.end()) ? lower->second.size() : upper->second.size();

    if (m_parameters.size() != nParams) {
        throw std::invalid_argument("Number of parameters must match number of node entries.");
    }

    unsigned nTerms = 1;
    if (interpolationType == Linear) {
        nTerms = 2;
    }
    PolynomialMap::OutMap coeffs;
    for (auto const& param : m_parameters) {
        coeffs[param].resize(nTerms);
    }
    if (interpolationType == Linear) {
        for (unsigned i = 0; i < nParams; ++i) {
            coeffs[m_parameters[i]][0] =
                (lower->second[i] - upper->second[i]) / (lower->first - upper->first);
            coeffs[m_parameters[i]][1] =
                (-upper->first * lower->second[i] + lower->first * upper->second[i]) /
                (lower->first - upper->first);
        }
    } else if (interpolationType == Upper) {
        for (unsigned i = 0; i < nParams; ++i) {
            coeffs[m_parameters[i]][0] = upper->second[i];
        }
    } else {
        for (unsigned i = 0; i < nParams; ++i) {
            coeffs[m_parameters[i]][0] = lower->second[i];
        }
    }

    PolynomialMap* model = new PolynomialMap;
    model->setMap(in, coeffs);

    AxisAlignedCuboidalDomainFilter::Limits limits;
    limits[*in.cbegin()] = std::pair<double, double>(lowerLimit, upperLimit);
    AxisAlignedCuboidalDomainFilter* domainFilter = new AxisAlignedCuboidalDomainFilter;
    domainFilter->setDomain(limits);
    domainFilter->add(model);

    return domainFilter;
}

} // namespace easi
