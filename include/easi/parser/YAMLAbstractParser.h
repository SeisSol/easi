#ifndef EASI_PARSER_YAMLABSTRACTPARSER_H_
#define EASI_PARSER_YAMLABSTRACTPARSER_H_

#include "easi/Component.h"

#include <set>
#include <string>
#include <yaml-cpp/yaml.h>

namespace easi {
class AsagiReader;
class YAMLAbstractParser {
  public:
  virtual ~YAMLAbstractParser() {}
  virtual Component* parse(const std::string& fileName) = 0;
  virtual Component* parse(const YAML::Node& node, const std::set<std::string>& in) = 0;
  virtual AsagiReader* asagiReader() = 0;
  virtual std::string currentFileName() = 0;
};

} // namespace easi

#endif
