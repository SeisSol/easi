/**
 * @file
 * This file is part of SeisSol.
 *
 * @author Carsten Uphoff (c.uphoff AT tum.de,
 *http://www5.in.tum.de/wiki/index.php/Carsten_Uphoff,_M.Sc.)
 *
 * @section LICENSE
 * Copyright (c) 2017, SeisSol Group
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @section DESCRIPTION
 **/
#ifndef EASI_COMPONENT_LAYEREDMODELBUILDER_H_
#define EASI_COMPONENT_LAYEREDMODELBUILDER_H_

#include "easi/component/Map.h"
#include "easi/util/Print.h"
#include <cmath>
#include <map>
#include <sstream>
#include <string>

namespace easi {
class LayeredModelBuilder {
public:
    enum InterpolationType { Lower, Upper, Linear };
    typedef std::map<double, std::vector<double>> Nodes;
    typedef std::vector<std::string> Parameters;

    void setMap(Map* map) { m_map = map; }
    void setInterpolationType(std::string const& interpolationType);
    void setInterpolationType(enum InterpolationType interpolationType) {
        m_interpolationType = interpolationType;
    }
    void setNodes(Nodes const& nodes);
    void setParameters(Parameters const& parameters) { m_parameters = parameters; }

    Component* getResult();

private:
    Component* createModel(Nodes::iterator& lower, Nodes::iterator& upper,
                           std::set<std::string> const& in);

    Map* m_map = nullptr;
    enum InterpolationType m_interpolationType;
    Nodes m_nodes;
    Parameters m_parameters;
};

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

#endif
