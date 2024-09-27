#include "easi/component/LuaMap.h"
#include <string>

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

#ifdef EASI_LUA_LMATHX
extern "C" int luaopen_mathx(lua_State* L);

// Lua changes a lot between the different versions, hence we need quite some ifdefs here
namespace {

static void loadLmathx(lua_State* L) {
#if LUA_VERSION_NUM < 502
    lua_pushcfunction(L, luaopen_mathx);
    lua_pushliteral(L, "mathx");
    lua_call(L, 1, 0);
#else
    luaL_requiref(L, "mathx", luaopen_mathx, 1);
    lua_pop(L, 1);
#endif
}

static std::string loadCodeLmathX() {
    // cf. example code for mathx: for Lua 5.3 and upwards, we need to supplement the math table.
    // (for 5.2 and lower, that's done automatically)
#if LUA_VERSION_NUM < 503
    return "";
#else
    return "for fname,fval in pairs(mathx) do math[fname] = fval end;\n";
#endif
    return "";
}

}
#else
namespace {
static void loadLmathx(lua_State* L) {
}

static std::string loadCodeLmathX() {
    return "";   
}
}
#endif

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
        
        loadLmathx(luaState);

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
            << " :::: f given by: " << function
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
    function = loadCodeLmathX() + newFunction;
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
