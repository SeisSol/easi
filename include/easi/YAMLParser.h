#ifndef EASI_YAMLPARSER_H_
#define EASI_YAMLPARSER_H_

#include "parser/YAMLAbstractParser.h"

#include <functional>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace easi {

class AsagiReader;
class Component;

class YAMLParser : public YAMLAbstractParser {
  public:
  using CreateFunction = Component*(const YAML::Node&,
                                    const std::set<std::string>&,
                                    YAMLAbstractParser*);
  template <typename T>
  using ParseFunction =
      void(T* component, const YAML::Node&, const std::set<std::string>&, YAMLAbstractParser*);

  YAMLParser(unsigned dimDomain,
             AsagiReader* externalAsagiReader = nullptr,
             char firstVariable = 'x');
  YAMLParser(const std::set<std::string>& variables, AsagiReader* externalAsagiReader = nullptr);
  virtual ~YAMLParser();

  void registerType(const std::string& tag, std::function<CreateFunction> create);
  template <typename T>
  void registerType(const std::string& tag, ParseFunction<T> parse) {
    auto create = [p = std::move(parse)](const YAML::Node& node,
                                         const std::set<std::string>& in,
                                         YAMLAbstractParser* parser) {
      T* component = new T;
      p(component, node, in, parser);
      return component;
    };
    registerType(tag, create);
  }

  Component* parse(const std::string& fileName) override;
  Component* parse(const YAML::Node& node, const std::set<std::string>& in) override;
  inline AsagiReader* asagiReader() override { return m_asagiReader; }
  inline std::string currentFileName() override { return m_currentFileName; }

  std::vector<std::string> getFileNameList();

  private:
  std::set<std::string> m_in;
  std::unordered_map<std::string, std::function<CreateFunction>> m_creators;
  AsagiReader* m_asagiReader;
  bool m_externalAsagiReader;
  std::string m_currentFileName;
  std::unordered_set<std::string> m_fileNames;
};

} // namespace easi

#endif // EASI_YAMLPARSER_H_
