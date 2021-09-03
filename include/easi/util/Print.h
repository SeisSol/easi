#ifndef EASI_UTIL_PRINT_H_
#define EASI_UTIL_PRINT_H_

#include <ostream>
#include <set>
#include <string>

namespace easi {

template <typename T>
void printWithSeparator(T const& vectorLike, std::ostream& os, std::string const& separator = ",") {
    auto it = vectorLike.cbegin();
    auto end = vectorLike.cend();
    if (it != end) {
        os << *it++;
    }
    while (it != end) {
        os << separator << *it++;
    }
}

} // namespace easi

#endif
