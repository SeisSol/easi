#ifndef EASI_COMPONENT_H_
#define EASI_COMPONENT_H_

#include "easi/Query.h"
#include "easi/ResultAdapter.h"
#include "easi/util/Slice.h"

#include <set>
#include <sstream>
#include <string>

namespace easi {

class Component {
public:
    virtual ~Component() {}

    virtual void evaluate(Query& query, ResultAdapter& result) = 0;
    virtual bool accept(int group, Slice<double> const& x) const = 0;
    virtual bool acceptAlways() const = 0;

    inline std::set<std::string> const& in() const { return m_in; }
    inline std::set<std::string> const& out() const { return m_out; }
    inline unsigned dimDomain() const { return m_in.size(); }
    inline unsigned dimCodomain() const { return m_out.size(); }

    inline void setFileReference(std::string const& fileReference) {
        m_fileReference = fileReference;
    }

    inline std::string addFileReference(std::string const& what_arg) {
        std::stringstream s;
        s << m_fileReference << ": " << what_arg;
        return s.str();
    }

    inline virtual std::set<std::string> suppliedParameters() { return out(); }

protected:
    inline void setIn(std::set<std::string> const& parameters) { m_in = parameters; }
    inline void setOut(std::set<std::string> const& parameters) { m_out = parameters; }

    std::string m_fileReference;

private:
    std::set<std::string> m_in;
    std::set<std::string> m_out;
};

} // namespace easi

#endif
