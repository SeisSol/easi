#ifndef EASI_LUAMAP_H
#define EASI_LUAMAP_H

#ifdef EASI_USE_LUA
#include <map>
#include <set>
#include <string>
#include <vector>



#include "easi/component/Map.h"
#include "easi/util/Matrix.h"

// Forward declaration to avoid including all Lua headers
struct lua_State;

namespace easi {
double getField(lua_State* L, const std::string& key);

class LuaMap : public Map {
public:
    void setMap(const std::set<std::string>& in,
                const std::set<std::string>& returns,
                const std::string& function);

    ~LuaMap() override;
protected:
    Matrix<double> map(Matrix<double>& x) override;

private:
    void executeLuaFunction(const Matrix<double>& x, Matrix<double>& y);
    std::string function;
    std::vector<std::string> idxToInputName;
    std::vector<std::string> idxToOutputName;
    std::vector<lua_State*> luaStates;
};

} // namespace easi

#endif // EASI_USE_LUA

#endif
