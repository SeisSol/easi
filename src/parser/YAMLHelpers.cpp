#include "easi/parser/YAMLHelpers.h"

namespace easi {

void checkExistence(YAML::Node const& node, std::string const& name) {
    std::stringstream ss;
    if (node.IsScalar() || !node[name]) {
        ss << node.Tag() << ": Parameter '" << name << "' is missing." << std::endl << node;
        throw YAML::Exception(node.Mark(), ss.str());
    }
}

void checkType(YAML::Node const& node, std::string const& name,
               std::set<YAML::NodeType::value> const& types, bool required) {
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
