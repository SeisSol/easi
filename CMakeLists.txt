project(easi)
cmake_minimum_required(VERSION 3.13)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

enable_language(CXX)
enable_language(Fortran)

option(ASAGI "Enable support for ASAGI" ON)
option(IMPALAJIT "Use ImpalaJIT" OFF)

### external packages ###

find_package(yaml-cpp 0.6 REQUIRED)
find_package(OpenMP)

if(IMPALAJIT)
    find_package (impalajit REQUIRED)
endif()

if(ASAGI)
    find_package (PkgConfig REQUIRED)
    find_package (HDF5 REQUIRED COMPONENTS C HL)
    pkg_check_modules (NETCDF REQUIRED IMPORTED_TARGET netcdf)
    pkg_check_modules (ASAGI REQUIRED IMPORTED_TARGET asagi)
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
target_link_libraries(easi
    PUBLIC
        yaml-cpp
)
if(${OpenMP_CXX_FOUND})
    target_link_libraries(easi PRIVATE OpenMP::OpenMP_CXX)
endif()
if(ASAGI)
    target_link_libraries(easi
        PRIVATE
            ${HDF5_C_HL_LIBRARIES} ${HDF5_C_LIBRARIES}
            PkgConfig::ASAGI
            PkgConfig::NETCDF
    )
    target_compile_definitions(easi PUBLIC -DUSE_ASAGI)
endif()
if(IMPALAJIT)
    target_link_libraries(easi PRIVATE
            impalajit::impalajit
    )
    target_compile_definitions(easi PUBLIC -DUSE_IMPALAJIT)
endif()

target_include_directories(easi
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# installation

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

configure_package_config_file(
    cmake/easiConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/easiConfig.cmake"
    INSTALL_DESTINATION ${CONFIG_DESTINATION}
)
install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/easiConfig.cmake"
    DESTINATION ${CONFIG_DESTINATION}
)


### easicube ###

if(ASAGI)
    add_executable (easicube tools/easicube.cpp)
    target_link_libraries(easicube PRIVATE easi)
    set_target_properties (easicube PROPERTIES
        INSTALL_RPATH_USE_LINK_PATH TRUE
    )
    install (TARGETS easicube)
endif()


### tests ###

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
easi_add_test(2_prem)
easi_add_test(3_layered_linear)
easi_add_test(5_function)
easi_add_test(26_function)
easi_add_test(33_layered_constant)
if(ASAGI)
    easi_add_test(101_asagi)
    easi_add_test(101_asagi_nearest)
endif ()
easi_add_test(f_16_scec)
easi_add_test(f_120_sumatra)
easi_add_test(supplied_parameters)
