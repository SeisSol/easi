#ifndef EASI_PARSER_YAMLABSTRACTPARSER_H_
#define EASI_PARSER_YAMLABSTRACTPARSER_H_

#include "easi/Component.h"

#include <yaml-cpp/yaml.h>

#include <set>
#include <string>

namespace easi {
class AsagiReader;
class YAMLAbstractParser {
public:
    virtual ~YAMLAbstractParser() {}
    virtual Component* parse(std::string const& fileName) = 0;
    virtual Component* parse(YAML::Node const& node, std::set<std::string> const& in) = 0;
    virtual AsagiReader* asagiReader() = 0;
    virtual std::string currentFileName() = 0;
};

} // namespace easi

#endif
