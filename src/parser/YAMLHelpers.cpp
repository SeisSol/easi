#include "easi/parser/YAMLHelpers.h"

namespace easi {

void checkExistence(const YAML::Node& node, const std::string& name) {
  std::stringstream ss;
  if (node.IsScalar() || !node[name]) {
    ss << node.Tag() << ": Parameter '" << name << "' is missing." << std::endl << node;
    throw YAML::Exception(node.Mark(), ss.str());
  }
}

void checkType(const YAML::Node& node,
               const std::string& name,
               const std::set<YAML::NodeType::value>& types,
               bool required) {
  std::stringstream ss;
  if (required) {
    checkExistence(node, name);
  }
  if (node[name] && types.find(node[name].Type()) == types.end()) {
    ss << node.Tag() << ": Parameter '" << name << "' has wrong type." << std::endl << node;
    throw YAML::Exception(node.Mark(), ss.str());
  }
}

} // namespace easi
