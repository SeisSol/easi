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

double LuaMap::executeLua(Matrix<double> x,
                          unsigned coordIdx,
                          unsigned funcIdx) {
    auto L = luaL_newstate();
    luaL_openlibs(L);
    const auto& script = functionStrings[funcIdx];
    const auto status = luaL_loadstring(L, script.data());
    if (status) {
        std::cerr
            << "Couldn't load script: "
            << lua_tostring(L, -1);
    }

    // Setup input vector
    lua_newtable(L);
    for (int i = 0; i < 3; ++i) {
        lua_pushnumber(L, i+1);
        lua_pushnumber(L, x(coordIdx, i));
        lua_rawset(L, -3);
    }
    lua_setglobal(L, "x");

    auto result = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (result != 0) {
        std::cerr
            << "error running script"
            << lua_tostring(L, -1)
            << std::endl;
    }

    double number = lua_tonumber(L, -1); // Get top of stack

    std::cout << number << std::endl;

    lua_pop(L, 1); // remove number from stack
    lua_close(L);

    return number;
}

Matrix<double> LuaMap::map(Matrix<double>& x) {

    assert(x.cols() == dimDomain());

    Matrix<double> y(x.rows(), dimCodomain());
    for (unsigned i = 0; i < y.rows(); ++i) {
        for (unsigned j = 0; j < y.cols(); ++j) {
            y(i, j) = executeLua(x, i, j);
        }
    }
    return y;
}

void LuaMap::setMap(std::set<std::string> const& in, OutMap const& functionMap) {
    functionStrings.clear();
    std::vector<std::string> functionDefinitions;

    for (const auto & it : functionMap) {
        functionStrings.push_back(it.second);
    }
    std::set<std::string> out;
    for (auto const& kv : functionMap) {
        out.insert(kv.first);
    }

    setIn(in);
    setOut(out);
}

} // namespace easi
#endif // EASI_USE_LUA