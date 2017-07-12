/**
 * @file
 * This file is part of SeisSol.
 *
 * @author Carsten Uphoff (c.uphoff AT tum.de, http://www5.in.tum.de/wiki/index.php/Carsten_Uphoff,_M.Sc.)
 *
 * @section LICENSE
 * Copyright (c) 2017, SeisSol Group
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @section DESCRIPTION
 **/
#ifndef EASI_UTIL_FUNCTIONWRAPPER_H_
#define EASI_UTIL_FUNCTIONWRAPPER_H_

#include <impalajit/types.hh>
#include "Matrix.h"

namespace easi {
typedef double (*function_wrapper_t)(dasm_gen_func, Matrix<double> const&, unsigned);

template<unsigned N> double function_wrapper(dasm_gen_func, Matrix<double> const&, unsigned);

template<> double function_wrapper<1>(dasm_gen_func f, Matrix<double> const& x, unsigned index) { return f(x(index, 0)); }
template<> double function_wrapper<2>(dasm_gen_func f, Matrix<double> const& x, unsigned index) { return f(x(index, 0), x(index, 1)); }
template<> double function_wrapper<3>(dasm_gen_func f, Matrix<double> const& x, unsigned index) { return f(x(index, 0), x(index, 1), x(index, 2)); }
template<> double function_wrapper<4>(dasm_gen_func f, Matrix<double> const& x, unsigned index) { return f(x(index, 0), x(index, 1), x(index, 2), x(index, 3)); }
template<> double function_wrapper<5>(dasm_gen_func f, Matrix<double> const& x, unsigned index) { return f(x(index, 0), x(index, 1), x(index, 2), x(index, 3), x(index, 4)); }
template<> double function_wrapper<6>(dasm_gen_func f, Matrix<double> const& x, unsigned index) { return f(x(index, 0), x(index, 1), x(index, 2), x(index, 3), x(index, 4), x(index, 5)); }
template<> double function_wrapper<7>(dasm_gen_func f, Matrix<double> const& x, unsigned index) { return f(x(index, 0), x(index, 1), x(index, 2), x(index, 3), x(index, 4), x(index, 5), x(index, 6)); }
template<> double function_wrapper<8>(dasm_gen_func f, Matrix<double> const& x, unsigned index) { return f(x(index, 0), x(index, 1), x(index, 2), x(index, 3), x(index, 4), x(index, 5), x(index, 6), x(index, 7)); }

function_wrapper_t getFunctionWrapper(unsigned dimDomain) {
  function_wrapper_t w;
  switch (dimDomain) {
    case 1: w = &function_wrapper<1>; break;
    case 2: w = &function_wrapper<2>; break;
    case 3: w = &function_wrapper<3>; break;
    case 4: w = &function_wrapper<4>; break;
    case 5: w = &function_wrapper<5>; break;
    case 6: w = &function_wrapper<6>; break;
    case 7: w = &function_wrapper<7>; break;
    case 8: w = &function_wrapper<8>; break;
    default: throw std::invalid_argument("Unsupported number of function arguments."); break; 
  }
  return w;
}
}

#endif
