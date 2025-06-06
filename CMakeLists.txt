cmake_minimum_required(VERSION 3.13)

project(easi)
set(EASI_VERSION_MAJOR 1)
set(EASI_VERSION_MINOR 5)
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
option(EASICUBE "Compile easicube" OFF)
option(PYTHON_BINDINGS "Compile python bindings" OFF)

option(LUA "Use Lua" ON)

function(check_parameter parameter_name value options)

    list(FIND options ${value} INDEX)

    set(WRONG_PARAMETER -1)
    if (${INDEX} EQUAL ${WRONG_PARAMETER})
        message(FATAL_ERROR "${parameter_name} is wrong. Specified \"${value}\". Allowed: ${options}")
    endif()

endfunction()

### external packages ###

find_package(yaml-cpp 0.6 REQUIRED)
find_package(OpenMP)

find_package(FILESYSTEM REQUIRED)

if (LUA)
    find_package(Lua REQUIRED)
endif()

if(ASAGI)
    find_package (HDF5 REQUIRED COMPONENTS C HL)
    find_package (NetCDF REQUIRED)

    find_package (asagi)
    if (NOT asagi_FOUND)
        find_package (PkgConfig REQUIRED)
        pkg_check_modules (ASAGI REQUIRED IMPORTED_TARGET asagi)
        add_library(asagi::asagi ALIAS PkgConfig::ASAGI)
    endif()
endif()


if(PYTHON_BINDINGS)
    set(PYBIND11_VER 2.13.6)
    # pybind11 is the first supporting compiling for NumPy 2
    set(PYBIND11_VER_MIN 2.12.0)
    set(PYBIND11_FINDPYTHON ON)

    set(NEED_FETCH FALSE)

    find_package(pybind11 QUIET)

    if(NOT pybind11_FOUND)
        set(NEED_FETCH TRUE)
    elseif(pybind11_VERSION VERSION_LESS PYBIND11_VER_MIN)
        set(NEED_FETCH TRUE)
        message(STATUS "Found pybind11 ${pybind11_VERSION} (in ${pybind11_DIR}) but need ${PYBIND11_VER_MIN}, will fetch newer version")
    endif()

    if(NEED_FETCH)
        include(FetchContent)
        FetchContent_Declare(pybind11
            GIT_REPOSITORY https://github.com/pybind/pybind11.git
            GIT_TAG v${PYBIND11_VER}
        )
        FetchContent_Populate(pybind11)

        # Manually add the downloaded pybind11 to search path
        list(PREPEND CMAKE_PREFIX_PATH "${pybind11_SOURCE_DIR}/cmake")
    endif()

    # Now, find_package again, cleanly
    find_package(pybind11 REQUIRED)

    message(STATUS "Building Python bindings enabled")
    add_subdirectory(python_bindings)
    # position-independent code is required for linking a dynamic lib (e.g. pybind)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()


### easi lib ###

set(EASI_SOURCES
    src/component/AffineMap.cpp
    src/component/AndersonianStress.cpp
    src/component/Composite.cpp
    src/component/ConstantMap.cpp
    src/component/DomainFilter.cpp
    src/component/EvalModel.cpp
    src/component/LuaMap.cpp
    src/component/LayeredModelBuilder.cpp
    src/component/OptimalStress.cpp
    src/component/PolynomialMap.cpp
    src/component/SCECFile.cpp
    src/component/Special.cpp
    src/component/Switch.cpp
    src/parser/YAMLComponentParsers.cpp
    src/parser/YAMLHelpers.cpp
    src/util/ImpalaHandler.cpp
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
target_compile_features(easi PUBLIC cxx_std_17)
set_target_properties(easi PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_compile_definitions(easi PUBLIC
    EASI_VERSION_MAJOR=${EASI_VERSION_MAJOR}
    EASI_VERSION_MINOR=${EASI_VERSION_MINOR}
    EASI_VERSION_PATCH=${EASI_VERSION_PATCH}
)

if ("${yaml-cpp_VERSION}" VERSION_GREATER_EQUAL "0.8")
  target_link_libraries(easi PUBLIC yaml-cpp::yaml-cpp)
elseif ("${yaml-cpp_VERSION}" VERSION_GREATER_EQUAL "0.7")
  target_link_libraries(easi PUBLIC yaml-cpp)
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

# avoid linking to the imported std::filesystem target for now; to avoid its export
target_link_libraries(easi PRIVATE ${FILESYSTEM_LIBRARIES})
if (NOT IS_STD_FILESYSTEM_SUPPORT)
    target_compile_definitions(easi PRIVATE EXPERIMENTAL_FS)
endif()

if(${OpenMP_CXX_FOUND})
    target_link_libraries(easi PRIVATE OpenMP::OpenMP_CXX)
endif()

if(ASAGI)
    find_package(MPI REQUIRED)
    target_link_libraries(easi
        PUBLIC
            asagi::asagi
            ${NetCDF_LIBRARY}
            ${HDF5_C_HL_LIBRARIES} ${HDF5_C_LIBRARIES}
    )
    target_include_directories(easi PUBLIC ${NetCDF_INCLUDE_DIRS})
    target_link_libraries(easi
            PUBLIC
            MPI::MPI_CXX
    )
    target_compile_definitions(easi PRIVATE EASI_USE_ASAGI)
endif()

if (LUA)
    target_include_directories(easi PRIVATE ${LUA_INCLUDE_DIR})
    target_link_libraries(easi PRIVATE ${LUA_LIBRARIES})
    target_compile_definitions(easi PRIVATE EASI_USE_LUA)
endif()

add_subdirectory(external)

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

    if(LUA)
        easi_add_test(2_prem)
        easi_add_test(5_function)
        easi_add_test(26_function)
        easi_add_test(f_16_scec)
        easi_add_test(f_120_sumatra)
        easi_add_test(supplied_parameters)
        easi_add_test(4_lua)
        easi_add_test(6_lua)
    endif()

    if(ASAGI)
        easi_add_test(101_asagi)
        easi_add_test(101_asagi_nearest)
    endif ()

endif()
