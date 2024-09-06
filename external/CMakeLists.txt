
if (LUA)

option(LUA_LMATHX "Compiles in Lua lmathx for more consistent math function support" ON)

if (LUA_LMATHX)

enable_language(C)

message("Enabled Lua lmathx support.")

if (LUA_VERSION_STRING VERSION_GREATER_EQUAL "5.5")
message(WARNING "You Lua version ${LUA_VERSION_STRING} is newer than the maximum lmathx package version supported (5.4.x). We'll try with the version compatible to 5.4/5.3, but no guarantee on success for that.")
target_sources(easi PRIVATE lua53/lmathx.c)
elseif (LUA_VERSION_STRING VERSION_GREATER_EQUAL "5.3")
target_sources(easi PRIVATE lua53/lmathx.c)
elseif (LUA_VERSION_STRING VERSION_GREATER_EQUAL "5.2")
target_sources(easi PRIVATE lua52/lmathx.c)
elseif (LUA_VERSION_STRING VERSION_GREATER_EQUAL "5.1")
target_sources(easi PRIVATE lua51/lmathx.c)
else()
message(FATAL_ERROR "You Lua version ${LUA_VERSION_STRING} is older than the minimum lmathx package version supported (5.1).")
endif()

target_compile_definitions(easi PRIVATE EASI_LUA_LMATHX)

endif()

endif()
