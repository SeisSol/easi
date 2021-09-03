#ifndef EASI_YAMLPARSER_H_
#define EASI_YAMLPARSER_H_

#include "parser/YAMLAbstractParser.h"

#include <yaml-cpp/yaml.h>

#include <functional>
#include <set>
#include <string>
#include <unordered_map>

namespace easi {

class AsagiReader;
class Component;

class YAMLParser : public YAMLAbstractParser {
public:
    using CreateFunction = Component*(YAML::Node const&, std::set<std::string> const&,
                                      YAMLAbstractParser*);
    template <typename T>
    using ParseFunction = void(T* component, YAML::Node const&, std::set<std::string> const&,
                               YAMLAbstractParser*);

    YAMLParser(unsigned dimDomain, AsagiReader* externalAsagiReader = nullptr,
               char firstVariable = 'x');
    virtual ~YAMLParser();

    void registerType(std::string const& tag, std::function<CreateFunction> create);
    template <typename T> void registerType(std::string const& tag, ParseFunction<T> parse) {
        auto create = [p = std::move(parse)](YAML::Node const& node,
                                             std::set<std::string> const& in,
                                             YAMLAbstractParser* parser) {
            T* component = new T;
            p(component, node, in, parser);
            return component;
        };
        registerType(tag, create);
    }

    Component* parse(std::string const& fileName) override;
    Component* parse(YAML::Node const& node, std::set<std::string> const& in) override;
    inline AsagiReader* asagiReader() override { return m_asagiReader; }
    inline std::string currentFileName() override { return m_currentFileName; }

private:
    std::set<std::string> m_in;
    std::unordered_map<std::string, std::function<CreateFunction>> m_creators;
    AsagiReader* m_asagiReader;
    bool m_externalAsagiReader;
    std::string m_currentFileName;
};

} // namespace easi

#endif // EASI_YAMLPARSER_H_
