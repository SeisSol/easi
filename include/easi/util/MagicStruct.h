#ifndef EASI_UTIL_MAGICSTRUCT_H_
#define EASI_UTIL_MAGICSTRUCT_H_

#include "Magic.h"
#include <algorithm>
#include <set>
#include <string>

namespace easi {

template <typename T> static std::string const* get_member_names();
template <typename T> static int get_binding_point(std::string const& member);
template <typename T> constexpr static int get_number_of_members();
template <typename T> static double T::*get_pointer_to_member(int bp);

template <typename T> static std::set<std::string> memberNamesToSet() {
    std::string const* begin = get_member_names<T>();
    std::string const* end = begin + get_number_of_members<T>();

    std::set<std::string> s;
    for (std::string const* it = begin; it != end; ++it) {
        s.insert(*it);
    }
    return s;
}

} // namespace easi

#define SAS_TOSTRING(STRUCT, X) #X
#define SAS_TOPOINTERTOMEMBER(STRUCT, X) &STRUCT::X
#define SELF_AWARE_STRUCT(STRUCT, ...)                                                             \
    namespace easi {                                                                               \
    template <> std::string const* get_member_names<STRUCT>() {                                    \
        static std::string const names[] = {                                                       \
            MAGIC_FOR_EACH_COMMA_SEPARATED(SAS_TOSTRING, STRUCT, __VA_ARGS__)};                    \
        return names;                                                                              \
    }                                                                                              \
    template <> int get_binding_point<STRUCT>(std::string const& field) {                          \
        std::string const* names = get_member_names<STRUCT>();                                     \
        std::string const* it = std::find(names, names + MAGIC_NARG(__VA_ARGS__), field);          \
        return (it != names + MAGIC_NARG(__VA_ARGS__)) ? std::distance(names, it) : -1;            \
    }                                                                                              \
    template <> constexpr int get_number_of_members<STRUCT>() { return MAGIC_NARG(__VA_ARGS__); }  \
    template <> double STRUCT::*get_pointer_to_member<STRUCT>(int bp) {                            \
        static double STRUCT::*const ptrToMember[] = {                                             \
            MAGIC_FOR_EACH_COMMA_SEPARATED(SAS_TOPOINTERTOMEMBER, STRUCT, __VA_ARGS__)};           \
        return ptrToMember[bp];                                                                    \
    }                                                                                              \
    }

#endif
