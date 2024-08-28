#ifndef EASI_UTIL_IMPALAHANDLER_H_
#define EASI_UTIL_IMPALAHANDLER_H_

#include <string>
#include <unordered_map>
#include <vector>

namespace easi {

std::string convertImpalaToLua(const std::unordered_map<std::string, std::string>& code, const std::vector<std::string>& in);

} // namespace easi

#endif // EASI_UTIL_IMPALAHANDLER_H_
