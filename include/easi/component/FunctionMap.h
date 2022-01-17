#ifndef EASI_COMPONENT_FUNCTIONMAP_H_
#define EASI_COMPONENT_FUNCTIONMAP_H_

#ifdef USE_IMPALAJIT
#include "easi/component/Map.h"
#include "easi/util/FunctionWrapper.h"
#include "easi/util/Matrix.h"

#include <impalajit/types.hh>

#include <map>
#include <set>
#include <string>
#include <vector>

namespace easi {
class FunctionMap : public Map {
public:
    typedef std::map<std::string, std::string> OutMap;

    virtual ~FunctionMap() {}

    void setMap(std::set<std::string> const& in, OutMap const& functionMap);

protected:
    virtual Matrix<double> map(Matrix<double>& x);

private:
    std::vector<dasm_gen_func> m_functions;
    function_wrapper_t m_functionWrapper;
};

} // namespace easi

#endif // USE_IMPALAJIT

#endif
