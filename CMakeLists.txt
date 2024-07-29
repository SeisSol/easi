project(easi)
cmake_minimum_required(VERSION 3.13)
set(EASI_VERSION_MAJOR 1)
set(EASI_VERSION_MINOR 3)
set(EASI_VERSION_PATCH 0)

add_compile_definitions(
    EASI_VERSION="${EASI_VERSION_MAJOR}.${EASI_VERSION_MINOR}.${EASI_VERSION_PATCH}"
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

enable_language(CXX)

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH} )

option(ASAGI "Enable support for ASAGI" ON)
option(BUILD_SHARED_LIBS "compile as a shared library" OFF)

option(TESTING "Compile tests" OFF)
option(EASICUBE "Compile easicube" ON)
option(IMPALAJIT "Use ImpalaJIT" ON)
option(PYTHON_BINDINGS "Compile python bindings" OFF)
set(IMPALAJIT_BACKEND original CACHE STRING "Which backend to use")
set(IMPALAJIT_BACKEND_OPTIONS original llvm)
set_property(CACHE IMPALAJIT_BACKEND PROPERTY STRINGS ${IMPALAJIT_BACKEND_OPTIONS})

option(LUA "Use Lua" ON)

function(check_parameter parameter_name value options)

    list(FIND options ${value} INDEX)

    set(WRONG_PARAMETER -1)
    if (${INDEX} EQUAL ${WRONG_PARAMETER})
        message(FATAL_ERROR "${parameter_name} is wrong. Specified \"${value}\". Allowed: ${options}")
    endif()

endfunction()
check_parameter("IMPALAJIT_BACKEND" "${IMPALAJIT_BACKEND}" "${IMPALAJIT_BACKEND_OPTIONS}")

### external packages ###

find_package(yaml-cpp 0.6 REQUIRED)
find_package(OpenMP)

find_package(FILESYSTEM REQUIRED)

if(IMPALAJIT)
    if (IMPALAJIT_BACKEND STREQUAL llvm)
        find_package(ImpalaJIT-LLVM 1.0 REQUIRED)
    else()
        find_package (impalajit REQUIRED)
    endif()
endif()

if (LUA)
    find_package(Lua REQUIRED)
endif()

if(ASAGI)
    find_package (PkgConfig REQUIRED)
    find_package (HDF5 REQUIRED COMPONENTS C HL)
    pkg_check_modules (NETCDF REQUIRED IMPORTED_TARGET netcdf)
    pkg_check_modules (ASAGI REQUIRED IMPORTED_TARGET asagi)
endif()

if (PYTHON_BINDINGS)
    set(PYBIND11_VER 2.11.1)
    set(PYBIND11_FINDPYTHON ON)
    find_package(pybind11 ${PYBIND11_VER} QUIET)
    if (NOT pybind11_FOUND)
         include(FetchContent)
         FetchContent_Declare(pybind11
                            GIT_REPOSITORY https://github.com/pybind/pybind11.git
                            GIT_TAG v${PYBIND11_VER})
         FetchContent_MakeAvailable(pybind11)
    endif ()

    message(STATUS "Building Python bindings enabled")
    add_subdirectory(python_bindings)
    # position-independent code is required for linking a dynamic lib (e.g. pybind)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

### easi lib ###

set(EASI_SOURCES
    src/component/AffineMap.cpp
    src/component/AndersonianStress.cpp
    src/component/Composite.cpp
    src/component/ConstantMap.cpp
    src/component/DomainFilter.cpp
    src/component/EvalModel.cpp
    src/component/FunctionMap.cpp
    src/component/LuaMap.cpp
    src/component/LayeredModelBuilder.cpp
    src/component/OptimalStress.cpp
    src/component/PolynomialMap.cpp
    src/component/SCECFile.cpp
    src/component/Special.cpp
    src/component/Switch.cpp
    src/parser/YAMLComponentParsers.cpp
    src/parser/YAMLHelpers.cpp
    src/util/FunctionWrapper.cpp
    src/util/RegularGrid.cpp
    src/Query.cpp
    src/YAMLParser.cpp
)
if(ASAGI)
    list(APPEND EASI_SOURCES
        src/component/ASAGI.cpp
        src/util/AsagiReader.cpp
    )
endif()

add_library(easi ${EASI_SOURCES})
target_compile_features(easi PUBLIC cxx_std_14)
set_target_properties(easi PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)


if ("${yaml-cpp_VERSION}" VERSION_GREATER_EQUAL "0.8")
  target_link_libraries(easi PUBLIC yaml-cpp::yaml-cpp)
else()
  # fallback code for old versions
  if (DEFINED YAML_CPP_INCLUDE_DIR AND EXISTS "${YAML_CPP_INCLUDE_DIR}")
    target_include_directories(easi PUBLIC ${YAML_CPP_INCLUDE_DIR})
  endif()
  if (DEFINED YAML_CPP_LIBRARIES)
    # use the YAML_CPP_LIBRARIES, if available (though it may just say `yaml-cpp`)
    target_link_libraries(easi PUBLIC ${YAML_CPP_LIBRARIES})
  else()
    # fallback
    target_link_libraries(easi PUBLIC yaml-cpp)
  endif()
endif()


target_link_libraries(easi PRIVATE std::filesystem)

if(${OpenMP_CXX_FOUND})
    target_link_libraries(easi PRIVATE OpenMP::OpenMP_CXX)
endif()

if(ASAGI)
    find_package(MPI REQUIRED)
    target_link_libraries(easi
        PUBLIC
            ${HDF5_C_HL_LIBRARIES} ${HDF5_C_LIBRARIES}
            PkgConfig::ASAGI
            PkgConfig::NETCDF
    )
    target_link_libraries(easi
            PUBLIC
            MPI::MPI_CXX
    )
    target_compile_definitions(easi PRIVATE EASI_USE_ASAGI)
endif()

if (IMPALAJIT)
    if (IMPALAJIT_BACKEND STREQUAL "llvm")
        target_link_libraries(easi PRIVATE llvm::impalajit)
    else()
        target_link_libraries(easi PRIVATE impalajit::impalajit)
    endif()
    target_compile_definitions(easi PRIVATE EASI_USE_IMPALAJIT)
endif()

if (LUA)
    target_include_directories(easi PRIVATE ${LUA_INCLUDE_DIR})
    target_link_libraries(easi PRIVATE ${LUA_LIBRARIES})
    target_compile_definitions(easi PRIVATE EASI_USE_LUA)
endif()

target_include_directories(easi
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# installation
set_target_properties(easi PROPERTIES INSTALL_RPATH_USE_LINK_PATH True)

set(CONFIG_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/easi)

install(TARGETS easi EXPORT easi-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT easi-targets
    FILE easiTargets.cmake
    NAMESPACE easi::
    DESTINATION ${CONFIG_DESTINATION}
)


# Replace variables in script
# Note: Configure file also replaces @PACKAGE_INIT@, we don't want that
set(PACKAGE_INIT_OLD "${PACKAGE_INIT}")
set(PACKAGE_INIT "@PACKAGE_INIT@")
configure_file(cmake/easiConfig.cmake.in cmake/easiConfig.cmake @ONLY)
set(PACKAGE_INIT "${PACKAGE_INIT_OLD}")
unset(PACKAGE_INIT_OLD)

configure_package_config_file(
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/easiConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/easiConfig.cmake"
    INSTALL_DESTINATION ${CONFIG_DESTINATION}
)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/easiConfigVersion.cmake"
        VERSION ${EASI_VERSION_MAJOR}.${EASI_VERSION_MINOR}.${EASI_VERSION_PATCH}
        COMPATIBILITY SameMajorVersion
)
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/easiConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/easiConfigVersion.cmake"
    DESTINATION ${CONFIG_DESTINATION}
)


### easicube ###

if(EASICUBE)
    if (ASAGI)
        add_executable (easicube tools/easicube.cpp)
        target_link_libraries(easicube PRIVATE easi)
        target_compile_definitions(easicube PRIVATE EASI_USE_ASAGI)
        set_target_properties (easicube PROPERTIES
            INSTALL_RPATH_USE_LINK_PATH TRUE
        )
        target_link_libraries(easicube PUBLIC ${HDF5_C_HL_LIBRARIES} ${HDF5_C_LIBRARIES} PkgConfig::ASAGI PkgConfig::NETCDF)
    
        install (TARGETS easicube)
    else()
        message(WARNING "easicube needs easi to be compiled with ASAGI. Since this is not the case, easicube will be skipped.")
    endif()
endif()

if (PYTHON_BINDINGS)
   #easi_INSTALL_PYTHONDIR can be predefined, e.g. with spack to install the python module with the other python modules
   set(INSTALL_DESTINATION "$<$<BOOL:${easi_INSTALL_PYTHONDIR}>:${easi_INSTALL_PYTHONDIR}/easi>${CONFIG_DESTINATION}/python_wrapper")
   install(TARGETS easi_python
           EXPORT easi_python
           LIBRARY DESTINATION ${INSTALL_DESTINATION})
endif()


### tests ###
if (TESTING)
    enable_language(Fortran)
    enable_testing()
    add_library(easitest
        tests/easitest.cpp
        tests/special.f90
    )
    target_link_libraries(easitest PUBLIC easi)

    function(easi_add_test name)
        add_executable(${name} tests/${name}.cpp)
        target_link_libraries(${name} PRIVATE easitest)
        string(TOUPPER ${name} NAME_UP)
        add_test(${NAME_UP}_TEST ${name} ${CMAKE_CURRENT_SOURCE_DIR}/examples/${name}.yaml)
    endfunction(easi_add_test)

    easi_add_test(0_constant)
    easi_add_test(1_groups)
    easi_add_test(3_layered_linear)
    easi_add_test(33_layered_constant)

    if(IMPALA)
        easi_add_test(2_prem)
        easi_add_test(5_function)
        easi_add_test(26_function)
        easi_add_test(f_16_scec)
        easi_add_test(f_120_sumatra)
        easi_add_test(supplied_parameters)
    endif()

    if(ASAGI)
        easi_add_test(101_asagi)
        easi_add_test(101_asagi_nearest)
    endif ()

    if (LUA)
        easi_add_test(4_lua)
        easi_add_test(6_lua)
    endif()

endif()
