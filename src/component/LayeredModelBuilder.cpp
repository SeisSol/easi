#include "easi/component/LayeredModelBuilder.h"
#include "easi/component/DomainFilter.h"
#include "easi/component/Map.h"
#include "easi/component/PolynomialMap.h"
#include "easi/util/Print.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>
#include <optional>
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
    
    std::optional<Nodes::iterator::value_type> previous = std::nullopt;
    for (const auto& node : m_nodes) {
        m_map->add(createModel(previous, node, m_map->out()));
        previous.emplace(node);
    }

    m_map->add(createModel(previous, std::nullopt, m_map->out()));

    return m_map;
}

Component* LayeredModelBuilder::createModel(const std::optional<Nodes::iterator::value_type>& lower,
                                            const std::optional<Nodes::iterator::value_type>& upper,
                                            std::set<std::string> const& in) {
    if (in.size() != 1) {
        std::ostringstream os;
        os << "Layered model requires 1D input (got ";
        printWithSeparator(in, os);
        os << ").";
        throw std::invalid_argument(os.str());
    }

    enum InterpolationType interpolationType;
    if (!lower.has_value()) {
        interpolationType = Upper;
    } else if (!upper.has_value()) {
        interpolationType = Lower;
    } else {
        interpolationType = m_interpolationType;
    }

    const double lowerLimit = lower.has_value() ? lower.value().first : -std::numeric_limits<double>::infinity();
    const double upperLimit = upper.has_value() ? upper.value().first : std::numeric_limits<double>::infinity();
    auto nParams = lower.has_value() ? lower.value().second.size() : upper.value().second.size();

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
                (lower.value().second[i] - upper.value().second[i]) / (lower.value().first - upper.value().first);
            coeffs[m_parameters[i]][1] =
                (-upper.value().first * lower.value().second[i] + lower.value().first * upper.value().second[i]) /
                (lower.value().first - upper.value().first);
        }
    } else if (interpolationType == Upper) {
        for (unsigned i = 0; i < nParams; ++i) {
            coeffs[m_parameters[i]][0] = upper.value().second[i];
        }
    } else {
        for (unsigned i = 0; i < nParams; ++i) {
            coeffs[m_parameters[i]][0] = lower.value().second[i];
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
