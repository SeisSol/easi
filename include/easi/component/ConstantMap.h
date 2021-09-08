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
#ifndef EASI_COMPONENT_CONSTANTMAP_H_
#define EASI_COMPONENT_CONSTANTMAP_H_

#include "easi/component/Map.h"
#include <map>
#include <string>

namespace easi {
class ConstantMap : public Map {
public:
    typedef std::map<std::string, double> OutMap;

    virtual ~ConstantMap() {}

    virtual Matrix<double> map(Matrix<double>& x);

    using Component::setIn; // Make setIn public
    void setMap(OutMap const& outMap);

private:
    std::vector<double> m_values;
};

Matrix<double> ConstantMap::map(Matrix<double>& x) {
    Matrix<double> y(x.rows(), dimCodomain());
    auto j = 0;
    for (auto const& value : m_values) {
        for (unsigned i = 0; i < x.rows(); ++i) {
            y(i, j) = value;
        }
        ++j;
    }

    return y;
}

void ConstantMap::setMap(OutMap const& outMap) {
    m_values.clear();
    std::set<std::string> out;
    for (auto const& kv : outMap) {
        out.insert(kv.first);
        m_values.push_back(kv.second);
    }
    setOut(out);

    assert(dimCodomain() == m_values.size());
}
} // namespace easi

#endif
