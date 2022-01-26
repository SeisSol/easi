#ifndef EASI_LUAMAP_H
#define EASI_LUAMAP_H

#ifdef EASI_USE_LUA
#include "easi/component/Map.h"
#include "easi/util/FunctionWrapper.h"
#include "easi/util/Matrix.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace easi {
class LuaMap : public Map {
public:
    typedef std::map<std::string, std::string> OutMap;

    virtual ~LuaMap() {}

    void setMap(std::set<std::string> const& in, OutMap const& functionMap);

protected:
    virtual Matrix<double> map(Matrix<double>& x);

private:
    double executeLua(Matrix<double> x, unsigned coordIdx, unsigned funcIdx);
    std::vector<std::string> functionStrings;
};

} // namespace easi

#endif // EASI_USE_LUA

#endif
