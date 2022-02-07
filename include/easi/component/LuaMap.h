#ifndef EASI_LUAMAP_H
#define EASI_LUAMAP_H

#ifdef EASI_USE_LUA
#include <map>
#include <set>
#include <string>
#include <vector>



#include "easi/component/Map.h"
#include "easi/util/FunctionWrapper.h"
#include "easi/util/Matrix.h"

struct lua_State;

namespace easi {
class LuaMap : public Map {
public:
    typedef std::map<std::string, std::string> OutMap;

    virtual ~LuaMap() {}

    void setMap(const std::set<std::string>& in,
                const std::set<std::string>& returns,
                const std::string& function);

protected:
    virtual Matrix<double> map(Matrix<double>& x);

private:
    double executeLuaFunction(Matrix<double> x, unsigned coordIdx, unsigned funcIdx);
    std::string function;
    std::vector<std::string> idxToNameMap;

    double getField(lua_State* L, const std::string& key);
};

} // namespace easi

#endif // EASI_USE_LUA

#endif
