##
# @file
# This file is part of SeisSol.
#
# @author Carsten Uphoff (c.uphoff AT tum.de, http://www5.in.tum.de/wiki/index.php/Carsten_Uphoff,_M.Sc.)
#
# @section LICENSE
# Copyright (c) 2017, SeisSol Group
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from this
#    software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# @section DESCRIPTION
# Builds easi unit tests.
#

project (easi)
cmake_minimum_required (VERSION 3.8)
enable_language (Fortran)

option (ASAGI "Enable support for ASAGI" ON)

# Compiler settings
add_compile_options (-Wall -Werror)
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# easi library
include_directories (include)

# external packages
find_package (PkgConfig REQUIRED)
pkg_check_modules (YAMLCPP REQUIRED IMPORTED_TARGET yaml-cpp)
pkg_check_modules (IMPALAJIT REQUIRED IMPORTED_TARGET impalajit)

if (${ASAGI})
    find_package (HDF5 REQUIRED COMPONENTS C HL)
    pkg_check_modules (NETCDF REQUIRED IMPORTED_TARGET netcdf)
    pkg_check_modules (ASAGI REQUIRED IMPORTED_TARGET asagi)
endif ()

find_package (OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

function (set_dependencies target)
    target_link_libraries (${target} PRIVATE
        PkgConfig::YAMLCPP
        PkgConfig::IMPALAJIT
    )
    if (${ASAGI})
        target_link_libraries (${target} PRIVATE
            ${HDF5_C_HL_LIBRARIES} ${HDF5_C_LIBRARIES}
            PkgConfig::ASAGI
            PkgConfig::NETCDF
        )
    endif ()
endfunction ()

function (easi_add_executable name files libs)
    add_executable (${name} ${files})
    set_dependencies (${name})
    target_link_libraries (${name} PRIVATE ${libs})
endfunction (easi_add_executable)

if (${ASAGI})
    easi_add_executable (easicube tools/easicube.cpp "")
endif ()

add_library (easitest tests/easitest.cpp tests/special.f90)
set_dependencies (easitest)
if (${ASAGI})
    add_definitions (-DUSE_ASAGI)
endif ()

enable_testing ()
function (easi_add_test name)
  easi_add_executable (${name} tests/${name}.cpp easitest)
  string (TOUPPER ${name} NAME_UP)
  add_test (${NAME_UP}_TEST ${name} ${CMAKE_CURRENT_SOURCE_DIR}/examples/${name}.yaml)
endfunction (easi_add_test)

easi_add_test (0_constant)
easi_add_test (1_groups)
easi_add_test (2_prem)
easi_add_test (3_layered_linear)
easi_add_test (5_function)
easi_add_test (26_function)
easi_add_test (33_layered_constant)
if (${ASAGI})
    easi_add_test (101_asagi)
    easi_add_test (101_asagi_nearest)
endif ()
easi_add_test (f_16_scec)
easi_add_test (f_120_sumatra)
easi_add_test (supplied_parameters)
