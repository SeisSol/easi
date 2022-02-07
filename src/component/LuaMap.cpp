#include "easi/component/LuaMap.h"

#ifdef EASI_USE_LUA
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
#include <cassert>
#include <iostream>
#include <ostream>
#include <stdexcept>

namespace easi {

double LuaMap::executeLuaFunction(Matrix<double> xin,
                                  unsigned coordIdx,
                                  unsigned funcIdx) {
    auto L = luaL_newstate();
    luaL_openlibs(L);
    const auto status = luaL_dostring(L, function.data());
    if (status) {
        std::cerr
            << "Couldn't load script: "
            << lua_tostring(L, -1);
        std::abort();
    }

    double z;

    /* push functions and arguments */
    lua_newtable(L);
    for (int i = 0; i < 3; ++i) {
        lua_pushnumber(L, i+1);
        lua_pushnumber(L, xin(coordIdx, i));
        lua_rawset(L, -3);
    }
    lua_pushstring(L, "test");
    lua_pushnumber(L, 42);
    lua_rawset(L, -3);
    lua_setglobal(L, "x");
    lua_getglobal(L, "f");  /* function to be called */
    lua_getglobal(L, "x");

    if (lua_pcall(L, 1, 1, 0) != 0) {
        std::cerr
        << "Error running function f "
        << lua_tostring(L, -1)
        << std::endl;
    }
    return getField(L, idxToNameMap[funcIdx]);
}

Matrix<double> LuaMap::map(Matrix<double>& x) {

    assert(x.cols() == dimDomain());

    Matrix<double> y(x.rows(), dimCodomain());
    for (unsigned i = 0; i < y.rows(); ++i) {
        for (unsigned j = 0; j < y.cols(); ++j) {
            y(i,j) = executeLuaFunction(x, i, j);
        }
    }
    return y;
}

void LuaMap::setMap(const std::set<std::string>& in,
                    const std::set<std::string>& out,
                    const std::string& newFunction) {
    setIn(in);
    setOut(out);
    function = newFunction;
    for (const auto& o : out) {
        idxToNameMap.push_back(o);
    }
}

double LuaMap::getField(lua_State* L, const std::string& key) {
    lua_pushstring(L, key.data());
    lua_gettable(L, -2);
    if (!lua_isnumber(L, -1)) {
        std::cerr
            << "Tried key "
            << key
            << " but it did not return a number."
            << std::endl;
    }
    const auto result = lua_tonumber(L, -1);
    lua_pop(L, 1);
    return result;
}

} // namespace easi
#endif // EASI_USE_LUA