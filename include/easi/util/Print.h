#ifndef EASI_UTIL_PRINT_H_
#define EASI_UTIL_PRINT_H_

#include <ostream>
#include <set>
#include <string>

namespace easi {

template <typename T>
void printWithSeparator(const T& vectorLike, std::ostream& os, const std::string& separator = ",") {
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
