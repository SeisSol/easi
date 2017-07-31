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
#ifndef EASI_UTIL_MAGICSTRUCT_H_
#define EASI_UTIL_MAGICSTRUCT_H_

#include <set>
#include <string>
#include <algorithm>
#include "Magic.h"

namespace easi {
  template<typename T> std::string const* get_member_names();
  template<typename T> int get_binding_point(std::string const& member);
  template<typename T> constexpr int get_number_of_members();
  template<typename T> double T::* get_pointer_to_member(int bp);
  
  template<typename T>
  std::set<std::string> memberNamesToSet() {
    std::string const* begin = get_member_names<T>();
    std::string const* end = begin + get_number_of_members<T>();

    std::set<std::string> s;
    for (std::string const* it = begin; it != end; ++it) {
      s.insert(*it);
    }
    return s;
  }
}

#define SAS_TOSTRING(STRUCT, X) #X
#define SAS_TOPOINTERTOMEMBER(STRUCT, X) &STRUCT::X
#define SELF_AWARE_STRUCT(STRUCT, ...) namespace easi { \  
                                          template<> std::string const* get_member_names<STRUCT>() { \
                                            static std::string const names[] = { \
                                              MAGIC_FOR_EACH_COMMA_SEPARATED(SAS_TOSTRING, STRUCT, __VA_ARGS__) \
                                            }; \
                                            return names; \
                                          } \
                                          template<> int get_binding_point<STRUCT>(std::string const& field) { \
                                            std::string const* names = get_member_names<STRUCT>(); \
                                            std::string const* it = std::find(names, names + MAGIC_NARG(__VA_ARGS__), field); \
                                            return (it != names + MAGIC_NARG(__VA_ARGS__)) ? std::distance(names, it) : -1; \
                                          } \
                                          template<> constexpr int get_number_of_members<STRUCT>() { return MAGIC_NARG(__VA_ARGS__); } \
                                          template<> double STRUCT::* get_pointer_to_member<STRUCT>(int bp) { \
                                            static double STRUCT::* const ptrToMember[] = { MAGIC_FOR_EACH_COMMA_SEPARATED(SAS_TOPOINTERTOMEMBER, STRUCT, __VA_ARGS__) }; \
                                            return ptrToMember[bp]; \
                                          } \
                                        }

#endif
