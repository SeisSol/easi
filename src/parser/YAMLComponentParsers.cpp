#include "easi/parser/YAMLComponentParsers.h"

#ifdef USE_ASAGI
#include "easi/util/AsagiReader.h"
#endif

#include <yaml-cpp/yaml.h>

namespace easi {

void parse_Component(Component* component, YAML::Node const& node, std::set<std::string> const&,
                     YAMLAbstractParser* parser) {
    std::stringstream s;
    s << parser->currentFileName() << "@" << node.Mark().line + 1;
    component->setFileReference(s.str());
}

void parse_Composite(Composite* component, YAML::Node const& node, std::set<std::string> const& in,
                     YAMLAbstractParser* parser) {
    if (node["components"]) {
        if (node["components"].IsSequence()) {
            for (YAML::const_iterator it = node["components"].begin();
                 it != node["components"].end(); ++it) {
                Component* child = parser->parse(*it, component->out());
                component->add(child);
            }
        } else {
            Component* child = parser->parse(node["components"], component->out());
            component->add(child);
        }
    }
    parse_Component(component, node, in, parser);
}

void parse_Switch(Switch* component, YAML::Node const& node, std::set<std::string> const& in,
                  YAMLAbstractParser* parser) {
    component->setInOut(in);
    for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
        Component* child = parser->parse(it->second, component->out());
        std::set<std::string> restrictions;
        if (it->first.IsSequence()) {
            std::vector<std::string> restrictionSeq = it->first.as<std::vector<std::string>>();
            restrictions.insert(restrictionSeq.begin(), restrictionSeq.end());
        } else {
            restrictions.insert(it->first.as<std::string>());
        }
        component->add(child, restrictions);
    }

    // Do not call Composite parser, as we don't want to inherit the "components" parameter
    parse_Component(component, node, in, parser);
}

void parse_Filter(Filter* component, YAML::Node const& node, std::set<std::string> const& in,
                  YAMLAbstractParser* parser) {
    parse_Composite(component, node, in, parser);
}

void parse_Any(Any* component, YAML::Node const& node, std::set<std::string> const& in,
               YAMLAbstractParser* parser) {
    component->setInOut(in);
    parse_Filter(component, node, in, parser);
}

void parse_GroupFilter(GroupFilter* component, YAML::Node const& node,
                       std::set<std::string> const& in, YAMLAbstractParser* parser) {
    checkType(node, "groups", {YAML::NodeType::Scalar, YAML::NodeType::Sequence});

    std::set<int> groups;
    if (node["groups"].IsScalar()) {
        groups.insert(node["groups"].as<int>());
    } else {
        std::vector<int> groupSeq = node["groups"].as<std::vector<int>>();
        groups.insert(groupSeq.begin(), groupSeq.end());
    }

    component->setInOut(in);
    component->setGroups(groups);
    parse_Filter(component, node, in, parser);
}

void parse_AxisAlignedCuboidalDomainFilter(AxisAlignedCuboidalDomainFilter* component,
                                           YAML::Node const& node, std::set<std::string> const& in,
                                           YAMLAbstractParser* parser) {
    checkType(node, "limits", {YAML::NodeType::Map});

    AxisAlignedCuboidalDomainFilter::Limits limits =
        node["limits"].as<AxisAlignedCuboidalDomainFilter::Limits>();
    component->setDomain(limits);

    parse_Filter(component, node, in, parser);
}

void parse_SphericalDomainFilter(SphericalDomainFilter* component, YAML::Node const& node,
                                 std::set<std::string> const& in, YAMLAbstractParser* parser) {
    checkType(node, "radius", {YAML::NodeType::Scalar});
    checkType(node, "centre", {YAML::NodeType::Map});

    double radius = node["radius"].as<double>();
    SphericalDomainFilter::Centre centre = node["centre"].as<SphericalDomainFilter::Centre>();

    component->setDomain(radius, centre);

    parse_Filter(component, node, in, parser);
}

void parse_Map(Map* component, YAML::Node const& node, std::set<std::string> const& in,
               YAMLAbstractParser* parser) {
    parse_Composite(component, node, in, parser);
}

void parse_ConstantMap(ConstantMap* component, YAML::Node const& node,
                       std::set<std::string> const& in, YAMLAbstractParser* parser) {
    checkType(node, "map", {YAML::NodeType::Map});

    ConstantMap::OutMap outMap = node["map"].as<ConstantMap::OutMap>();

    component->setIn(in);
    component->setMap(outMap);
    parse_Map(component, node, in, parser);
}

void parse_AffineMap(AffineMap* component, YAML::Node const& node, std::set<std::string> const& in,
                     YAMLAbstractParser* parser) {
    checkType(node, "matrix", {YAML::NodeType::Map});
    checkType(node, "translation", {YAML::NodeType::Map});

    AffineMap::Transformation matrix = node["matrix"].as<AffineMap::Transformation>();
    AffineMap::Translation translation = node["translation"].as<AffineMap::Translation>();

    component->setMap(in, matrix, translation);

    parse_Map(component, node, in, parser);
}

void parse_FunctionMap(FunctionMap* component, YAML::Node const& node,
                       std::set<std::string> const& in, YAMLAbstractParser* parser) {
    checkType(node, "map", {YAML::NodeType::Map});

    FunctionMap::OutMap out = node["map"].as<FunctionMap::OutMap>();

    component->setMap(in, out);
    parse_Map(component, node, in, parser);
}

void parse_PolynomialMap(PolynomialMap* component, YAML::Node const& node,
                         std::set<std::string> const& in, YAMLAbstractParser* parser) {
    checkType(node, "map", {YAML::NodeType::Map});

    PolynomialMap::OutMap coeffs = node["map"].as<PolynomialMap::OutMap>();
    component->setMap(in, coeffs);

    parse_Map(component, node, in, parser);
}

template <typename GridImpl>
void parse_Grid(Grid<GridImpl>* component, YAML::Node const& node, std::set<std::string> const& in,
                YAMLAbstractParser* parser) {
    if (node["interpolation"]) {
        std::string interpolation = node["interpolation"].as<std::string>();
        component->setInterpolationType(interpolation);
    }

    parse_Map(component, node, in, parser);
}

void parse_SCECFile(SCECFile* component, YAML::Node const& node, std::set<std::string> const& in,
                    YAMLAbstractParser* parser) {
    checkType(node, "file", {YAML::NodeType::Scalar});

    std::string fileName = node["file"].as<std::string>();
    component->setMap(in, fileName);

    parse_Grid<SCECFile>(component, node, in, parser);
}

#ifdef USE_ASAGI
void parse_ASAGI(ASAGI* component, YAML::Node const& node, std::set<std::string> const& in,
                 YAMLAbstractParser* parser) {
    checkType(node, "file", {YAML::NodeType::Scalar});
    checkType(node, "parameters", {YAML::NodeType::Sequence});

    auto parameters = node["parameters"].as<std::vector<std::string>>();

    std::string varName = "data";
    if (node["var"]) {
        varName = node["var"].as<std::string>();
    }

    std::string fileName = node["file"].as<std::string>();
    asagi::Grid* grid = parser->asagiReader()->open(fileName.c_str(), varName.c_str());

    component->setGrid(in, parameters, grid, parser->asagiReader()->numberOfThreads());

    parse_Grid<ASAGI>(component, node, in, parser);
}
#endif

void parse_EvalModel(EvalModel* component, YAML::Node const& node, std::set<std::string> const& in,
                     YAMLAbstractParser* parser) {
    checkExistence(node, "model");
    checkType(node, "parameters", {YAML::NodeType::Sequence});

    Component* model = parser->parse(node["model"], in);
    auto parameters = node["parameters"].as<std::vector<std::string>>();
    std::set<std::string> out;
    for (auto const& p : parameters) {
        out.insert(p);
    }

    component->setModel(in, out, model);

    parse_Map(component, node, in, parser);
}

Component* create_LayeredModel(YAML::Node const& node, std::set<std::string> const& in,
                               YAMLAbstractParser* parser) {
    LayeredModelBuilder builder;

    checkExistence(node, "map");
    checkType(node, "interpolation", {YAML::NodeType::Scalar});
    checkType(node, "nodes", {YAML::NodeType::Map});
    checkType(node, "parameters", {YAML::NodeType::Sequence});

    Component* rawMap = parser->parse(node["map"], in);
    Map* map = upcast<Map>(node, rawMap);
    std::string interpolation = node["interpolation"].as<std::string>();
    LayeredModelBuilder::Nodes nodes = node["nodes"].as<LayeredModelBuilder::Nodes>();
    LayeredModelBuilder::Parameters parameters =
        node["parameters"].as<LayeredModelBuilder::Parameters>();

    builder.setMap(map);
    builder.setInterpolationType(interpolation);
    builder.setNodes(nodes);
    builder.setParameters(parameters);

    return builder.getResult();
}

Component* create_Include(YAML::Node const& node, std::set<std::string> const& in,
                          YAMLAbstractParser* parser) {
    return parser->parse(node.as<std::string>());
}

} // namespace easi
