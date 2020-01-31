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
#ifndef EASI_UTIL_MAGIC_H_
#define EASI_UTIL_MAGIC_H_
#define MAGIC_EXPAND(X) X

#define MAGIC_CONCATENATE(a,b) MAGIC_CONCATENATE_1(a,b)
#define MAGIC_CONCATENATE_1(a,b) MAGIC_CONCATENATE_2(a,b)
#define MAGIC_CONCATENATE_2(a,b) a##b

#define MAGIC_NARG(...) MAGIC_NARG_(__VA_ARGS__, MAGIC_RSEQ_N())
#define MAGIC_NARG_(...) MAGIC_EXPAND(MAGIC_ARG_N(__VA_ARGS__))
#define MAGIC_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, N, ...) N
#define MAGIC_RSEQ_N() 20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0

#define MAGIC_FOR_EACH_1(N, macro, prefix, first) macro(N-1, prefix, first)
#define MAGIC_FOR_EACH_2(N, macro, prefix, first, ...) macro(N-2, prefix, first) MAGIC_EXPAND(MAGIC_FOR_EACH_1(N, macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_3(N, macro, prefix, first, ...) macro(N-3, prefix, first) MAGIC_EXPAND(MAGIC_FOR_EACH_2(N, macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_4(N, macro, prefix, first, ...) macro(N-4, prefix, first) MAGIC_EXPAND(MAGIC_FOR_EACH_3(N, macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_5(N, macro, prefix, first, ...) macro(N-5, prefix, first) MAGIC_EXPAND(MAGIC_FOR_EACH_4(N, macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_6(N, macro, prefix, first, ...) macro(N-6, prefix, first) MAGIC_EXPAND(MAGIC_FOR_EACH_5(N, macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_7(N, macro, prefix, first, ...) macro(N-7, prefix, first) MAGIC_EXPAND(MAGIC_FOR_EACH_6(N, macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_8(N, macro, prefix, first, ...) macro(N-8, prefix, first) MAGIC_EXPAND(MAGIC_FOR_EACH_7(N, macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_9(N, macro, prefix, first, ...) macro(N-9, prefix, first) MAGIC_EXPAND(MAGIC_FOR_EACH_8(N, macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_10(N, macro, prefix, first, ...) macro(N-10, prefix, first) MAGIC_EXPAND(MAGIC_FOR_EACH_9(N, macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_11(N, macro, prefix, first, ...) macro(N-11, prefix, first) MAGIC_EXPAND(MAGIC_FOR_EACH_10(N, macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_12(N, macro, prefix, first, ...) macro(N-12, prefix, first) MAGIC_EXPAND(MAGIC_FOR_EACH_11(N, macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_13(N, macro, prefix, first, ...) macro(N-13, prefix, first) MAGIC_EXPAND(MAGIC_FOR_EACH_12(N, macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_14(N, macro, prefix, first, ...) macro(N-14, prefix, first) MAGIC_EXPAND(MAGIC_FOR_EACH_13(N, macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_15(N, macro, prefix, first, ...) macro(N-15, prefix, first) MAGIC_EXPAND(MAGIC_FOR_EACH_14(N, macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_16(N, macro, prefix, first, ...) macro(N-16, prefix, first) MAGIC_EXPAND(MAGIC_FOR_EACH_15(N, macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_17(N, macro, prefix, first, ...) macro(N-17, prefix, first) MAGIC_EXPAND(MAGIC_FOR_EACH_16(N, macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_18(N, macro, prefix, first, ...) macro(N-18, prefix, first) MAGIC_EXPAND(MAGIC_FOR_EACH_17(N, macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_19(N, macro, prefix, first, ...) macro(N-19, prefix, first) MAGIC_EXPAND(MAGIC_FOR_EACH_18(N, macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_20(N, macro, prefix, first, ...) macro(N-20, prefix, first) MAGIC_EXPAND(MAGIC_FOR_EACH_19(N, macro, prefix, __VA_ARGS__))

#define MAGIC_FOR_EACH_COMMA_SEPARATED_1(macro, prefix, first) macro(prefix, first)
#define MAGIC_FOR_EACH_COMMA_SEPARATED_2(macro, prefix, first, ...) macro(prefix, first), MAGIC_EXPAND(MAGIC_FOR_EACH_COMMA_SEPARATED_1(macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_COMMA_SEPARATED_3(macro, prefix, first, ...) macro(prefix, first), MAGIC_EXPAND(MAGIC_FOR_EACH_COMMA_SEPARATED_2(macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_COMMA_SEPARATED_4(macro, prefix, first, ...) macro(prefix, first), MAGIC_EXPAND(MAGIC_FOR_EACH_COMMA_SEPARATED_3(macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_COMMA_SEPARATED_5(macro, prefix, first, ...) macro(prefix, first), MAGIC_EXPAND(MAGIC_FOR_EACH_COMMA_SEPARATED_4(macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_COMMA_SEPARATED_6(macro, prefix, first, ...) macro(prefix, first), MAGIC_EXPAND(MAGIC_FOR_EACH_COMMA_SEPARATED_5(macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_COMMA_SEPARATED_7(macro, prefix, first, ...) macro(prefix, first), MAGIC_EXPAND(MAGIC_FOR_EACH_COMMA_SEPARATED_6(macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_COMMA_SEPARATED_8(macro, prefix, first, ...) macro(prefix, first), MAGIC_EXPAND(MAGIC_FOR_EACH_COMMA_SEPARATED_7(macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_COMMA_SEPARATED_9(macro, prefix, first, ...) macro(prefix, first), MAGIC_EXPAND(MAGIC_FOR_EACH_COMMA_SEPARATED_8(macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_COMMA_SEPARATED_10(macro, prefix, first, ...) macro(prefix, first), MAGIC_EXPAND(MAGIC_FOR_EACH_COMMA_SEPARATED_9(macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_COMMA_SEPARATED_11(macro, prefix, first, ...) macro(prefix, first), MAGIC_EXPAND(MAGIC_FOR_EACH_COMMA_SEPARATED_10(macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_COMMA_SEPARATED_12(macro, prefix, first, ...) macro(prefix, first), MAGIC_EXPAND(MAGIC_FOR_EACH_COMMA_SEPARATED_11(macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_COMMA_SEPARATED_13(macro, prefix, first, ...) macro(prefix, first), MAGIC_EXPAND(MAGIC_FOR_EACH_COMMA_SEPARATED_12(macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_COMMA_SEPARATED_14(macro, prefix, first, ...) macro(prefix, first), MAGIC_EXPAND(MAGIC_FOR_EACH_COMMA_SEPARATED_13(macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_COMMA_SEPARATED_15(macro, prefix, first, ...) macro(prefix, first), MAGIC_EXPAND(MAGIC_FOR_EACH_COMMA_SEPARATED_14(macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_COMMA_SEPARATED_16(macro, prefix, first, ...) macro(prefix, first), MAGIC_EXPAND(MAGIC_FOR_EACH_COMMA_SEPARATED_15(macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_COMMA_SEPARATED_17(macro, prefix, first, ...) macro(prefix, first), MAGIC_EXPAND(MAGIC_FOR_EACH_COMMA_SEPARATED_16(macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_COMMA_SEPARATED_18(macro, prefix, first, ...) macro(prefix, first), MAGIC_EXPAND(MAGIC_FOR_EACH_COMMA_SEPARATED_17(macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_COMMA_SEPARATED_19(macro, prefix, first, ...) macro(prefix, first), MAGIC_EXPAND(MAGIC_FOR_EACH_COMMA_SEPARATED_18(macro, prefix, __VA_ARGS__))
#define MAGIC_FOR_EACH_COMMA_SEPARATED_20(macro, prefix, first, ...) macro(prefix, first), MAGIC_EXPAND(MAGIC_FOR_EACH_COMMA_SEPARATED_19(macro, prefix, __VA_ARGS__))

#define MAGIC_FOR_EACH_(N, macro, prefix, ...) MAGIC_CONCATENATE(MAGIC_FOR_EACH_, N)(N, macro, prefix, __VA_ARGS__)
#define MAGIC_FOR_EACH(macro, prefix, ...) MAGIC_FOR_EACH_(MAGIC_NARG(__VA_ARGS__), macro, prefix, __VA_ARGS__)

#define MAGIC_FOR_EACH_COMMA_SEPARATED_(N, macro, prefix, ...) MAGIC_CONCATENATE(MAGIC_FOR_EACH_COMMA_SEPARATED_, N)(macro, prefix, __VA_ARGS__)
#define MAGIC_FOR_EACH_COMMA_SEPARATED(macro, prefix, ...) MAGIC_FOR_EACH_COMMA_SEPARATED_(MAGIC_NARG(__VA_ARGS__), macro, prefix, __VA_ARGS__)

#endif
