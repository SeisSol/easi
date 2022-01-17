#ifndef EASI_PARSER_YAMLCOMPONENTPARSERS_H_
#define EASI_PARSER_YAMLCOMPONENTPARSERS_H_

#include "easi/component/AffineMap.h"
#include "easi/component/AndersonianStress.h"
#include "easi/component/Any.h"
#include "easi/component/ConstantMap.h"
#include "easi/component/DomainFilter.h"
#include "easi/component/EvalModel.h"
#include "easi/component/FunctionMap.h"
#include "easi/component/GroupFilter.h"
#include "easi/component/LayeredModelBuilder.h"
#include "easi/component/OptimalStress.h"
#include "easi/component/PolynomialMap.h"
#include "easi/component/SCECFile.h"
#include "easi/component/SpecialMap.h"
#include "easi/component/Switch.h"
#include "easi/parser/YAMLAbstractParser.h"
#include "easi/parser/YAMLHelpers.h"

#ifdef USE_ASAGI
#include "easi/component/ASAGI.h"
#endif

#include <yaml-cpp/yaml.h>

#include <map>
#include <set>
#include <string>

namespace easi {

void parse_Component(Component* component, YAML::Node const& node, std::set<std::string> const&,
                     YAMLAbstractParser* parser);
void parse_Composite(Composite* component, YAML::Node const& node, std::set<std::string> const& in,
                     YAMLAbstractParser* parser);
void parse_Switch(Switch* component, YAML::Node const& node, std::set<std::string> const& in,
                  YAMLAbstractParser* parser);
void parse_Filter(Filter* component, YAML::Node const& node, std::set<std::string> const& in,
                  YAMLAbstractParser* parser);
void parse_Any(Any* component, YAML::Node const& node, std::set<std::string> const& in,
               YAMLAbstractParser* parser);
void parse_GroupFilter(GroupFilter* component, YAML::Node const& node,
                       std::set<std::string> const& in, YAMLAbstractParser* parser);
void parse_AxisAlignedCuboidalDomainFilter(AxisAlignedCuboidalDomainFilter* component,
                                           YAML::Node const& node, std::set<std::string> const& in,
                                           YAMLAbstractParser* parser);
void parse_SphericalDomainFilter(SphericalDomainFilter* component, YAML::Node const& node,
                                 std::set<std::string> const& in, YAMLAbstractParser* parser);
void parse_Map(Map* component, YAML::Node const& node, std::set<std::string> const& in,
               YAMLAbstractParser* parser);
void parse_ConstantMap(ConstantMap* component, YAML::Node const& node,
                       std::set<std::string> const& in, YAMLAbstractParser* parser);
void parse_AffineMap(AffineMap* component, YAML::Node const& node, std::set<std::string> const& in,
                     YAMLAbstractParser* parser);
#ifdef USE_IMPALAJIT
void parse_FunctionMap(FunctionMap* component, YAML::Node const& node,
                       std::set<std::string> const& in, YAMLAbstractParser* parser);
#endif
void parse_PolynomialMap(PolynomialMap* component, YAML::Node const& node,
                         std::set<std::string> const& in, YAMLAbstractParser* parser);
void parse_SCECFile(SCECFile* component, YAML::Node const& node, std::set<std::string> const& in,
                    YAMLAbstractParser* parser);
#ifdef USE_ASAGI
void parse_ASAGI(ASAGI* component, YAML::Node const& node, std::set<std::string> const& in,
                 YAMLAbstractParser* parser);
#endif

void parse_EvalModel(EvalModel* component, YAML::Node const& node, std::set<std::string> const& in,
                     YAMLAbstractParser* parser);

Component* create_LayeredModel(YAML::Node const& node, std::set<std::string> const& in,
                               YAMLAbstractParser* parser);
Component* create_Include(YAML::Node const& node, std::set<std::string> const& in,
                          YAMLAbstractParser* parser);

template <typename Special>
Component* create_Special(YAML::Node const& node, std::set<std::string> const& in,
                          YAMLAbstractParser* parser) {
    checkType(node, "constants", {YAML::NodeType::Map}, false);

    std::map<std::string, double> constants;
    if (node["constants"]) {
        constants = node["constants"].as<std::map<std::string, double>>();
    }

    SpecialMap<Special>* component = new SpecialMap<Special>;
    component->setMap(constants);

    parse_Map(component, node, in, parser);

    return component;
}

} // namespace easi

#endif
