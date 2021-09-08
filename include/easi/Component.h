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
#ifndef EASI_COMPONENT_H_
#define EASI_COMPONENT_H_

#include "easi/Query.h"
#include "easi/ResultAdapter.h"
#include <sstream>

namespace easi {
class Component {
public:
    virtual ~Component() {}

    virtual void evaluate(Query& query, ResultAdapter& result) = 0;
    virtual bool accept(int group, Slice<double> const& x) const = 0;
    virtual bool acceptAlways() const = 0;

    std::set<std::string> const& in() const { return m_in; }
    std::set<std::string> const& out() const { return m_out; }
    unsigned dimDomain() const { return m_in.size(); }
    unsigned dimCodomain() const { return m_out.size(); }

    void setFileReference(std::string const& fileReference) { m_fileReference = fileReference; }

    std::string addFileReference(std::string const& what_arg) {
        std::stringstream s;
        s << m_fileReference << ": " << what_arg;
        return s.str();
    }

    virtual std::set<std::string> suppliedParameters() { return out(); }

protected:
    void setIn(std::set<std::string> const& parameters) { m_in = parameters; }
    void setOut(std::set<std::string> const& parameters) { m_out = parameters; }

    std::string m_fileReference;

private:
    std::set<std::string> m_in;
    std::set<std::string> m_out;
};
} // namespace easi

#endif
