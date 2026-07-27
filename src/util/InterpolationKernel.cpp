#include "easi/util/InterpolationKernel.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <string>

namespace easi {

InterpolationType interpolationTypeFromString(const std::string& name) {
  std::string lower = name;
  std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });

  if (lower == "nearest") {
    return InterpolationType::Nearest;
  }
  if (lower == "linear") {
    return InterpolationType::Linear;
  }
  if (lower == "cubic") {
    return InterpolationType::Cubic;
  }

  std::ostringstream os;
  os << "Invalid interpolation type " << name << " (expected one of nearest, linear, cubic).";
  throw std::invalid_argument(os.str());
}

const char* interpolationTypeToString(InterpolationType type) {
  switch (type) {
  case InterpolationType::Nearest:
    return "nearest";
  case InterpolationType::Linear:
    return "linear";
  case InterpolationType::Cubic:
    return "cubic";
  }
  return "unknown";
}

} // namespace easi
