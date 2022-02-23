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

double LuaMap::executeLuaFunction(Matrix<double> x,
                                  unsigned coordIdx,
                                  unsigned funcIdx) {
    if (!luaState) {
        luaState = luaL_newstate();
        luaL_openlibs(luaState);
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


    // Push function and arguments to stack
    lua_getglobal(luaState, "f");  // the function

    // Add table as input: x holds coordinates
    lua_newtable(luaState);
    for (int i = 0; i < x.cols(); ++i) {
        // Support x[1] indexing
        lua_pushnumber(luaState, i+1);
        lua_pushnumber(luaState, x(coordIdx, i));
        lua_rawset(luaState, -3);

        // Support x["x"] indexing
        lua_pushstring(luaState, idxToInputName[i].data());
        lua_pushnumber(luaState, x(coordIdx, i));
        lua_rawset(luaState, -3);
    }

    if (lua_pcall(luaState, 1, 1, 0) != 0) {
        std::cerr
        << "Error running function f "
        << lua_tostring(luaState, -1)
        << std::endl;
        std::abort();
    }

    const auto retVal = getField(luaState, idxToOutputName[funcIdx]);

    // Reset stack size to value before function call
    // This should avoid stack overflows
    lua_settop(luaState, top); 

    return retVal;
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
