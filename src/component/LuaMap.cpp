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

double getField(lua_State* L, const std::string& key) {
    lua_pushstring(L, key.data());
    lua_gettable(L, -2);
    if (!lua_isnumber(L, -1)) {
        std::cerr
            << "Tried key "
            << key
            << " but it did not return a number."
            << std::endl;
        std::abort();
    }
    const auto result = lua_tonumber(L, -1);
    lua_pop(L, 1);
    return result;
}

void LuaMap::executeLuaFunction(const Matrix<double>& x,
                                  Matrix<double>& y) {
    if (!luaState) {
        luaState = luaL_newstate();
        luaL_openlibs(luaState);

        // luaJIT_setmode(luaState, -1, LUAJIT_MODE_ALLFUNC|LUAJIT_MODE_ON);

        const auto status = luaL_dostring(luaState, function.data());
        if (status) {
            std::cerr
                << "Couldn't load script: "
                << lua_tostring(luaState, -1);
            std::abort();
        }
    }
    
    // Save stack size
    const auto top = lua_gettop(luaState);

    for (unsigned row = 0; row < y.rows(); ++row) {
        // Push function and arguments to stack
        lua_getglobal(luaState, "f");  // the function

        // Add table as input: x holds coordinates
        lua_newtable(luaState);

        for (int i = 0; i < x.cols(); ++i) {
            // Support x[1] indexing
            lua_pushnumber(luaState, i+1);
            lua_pushnumber(luaState, x(row, i));
            lua_rawset(luaState, -3);

            // Support x["x"] indexing
            lua_pushstring(luaState, idxToInputName[i].data());
            lua_pushnumber(luaState, x(row, i));
            lua_rawset(luaState, -3);
        }

        if (lua_pcall(luaState, 1, 1, 0) != 0) {
            std::cerr
            << "Error running function f "
            << lua_tostring(luaState, -1)
            << std::endl;
            std::abort();
        }

        for (unsigned col = 0; col < y.cols(); ++col) {
            y(row, col) = getField(luaState, idxToOutputName[col]);
        }

        // Reset stack size to value before function call
        // This should avoid stack overflows
        lua_settop(luaState, top);
    }
}

Matrix<double> LuaMap::map(Matrix<double>& x) {

    assert(x.cols() == dimDomain());

    Matrix<double> y(x.rows(), dimCodomain());
    executeLuaFunction(x, y);
    return y;
}

void LuaMap::setMap(const std::set<std::string>& in,
                    const std::set<std::string>& out,
                    const std::string& newFunction) {
    setIn(in);
    setOut(out);
    function = newFunction;
    for (const auto& i: in) {
        idxToInputName.push_back(i);
    }
    for (const auto& o : out) {
        idxToOutputName.push_back(o);
    }
}

LuaMap::~LuaMap() {
   if (luaState) {
       lua_close(luaState);
   }
}



} // namespace easi
#endif // EASI_USE_LUA
