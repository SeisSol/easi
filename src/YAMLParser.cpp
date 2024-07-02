#include "easi/YAMLParser.h"
#include "easi/Component.h"
#include "easi/component/AndersonianStress.h"
#include "easi/component/OptimalStress.h"
#include "easi/component/Special.h"
#include "easi/parser/YAMLComponentParsers.h"

#ifdef EASI_USE_ASAGI
#include "easi/util/AsagiReader.h"
#else
namespace easi {
class AsagiReader {};
} // namespace easi
#endif

#include <iostream>

#ifdef EXPERIMENTAL_FS
#include <experimental/filesystem>
#else
#include <filesystem>
#endif

#ifdef EXPERIMENTAL_FS
namespace fs = std::experimental::filesystem;
#else
namespace fs = std::filesystem;
#endif

namespace easi {

YAMLParser::YAMLParser(unsigned dimDomain, AsagiReader* externalAsagiReader, char firstVariable)
    : m_asagiReader(externalAsagiReader), m_externalAsagiReader(externalAsagiReader != nullptr) {
    for (unsigned i = 0; i < dimDomain; ++i) {
        m_in.insert(std::string(1, firstVariable + i));
    }
    registerType("!Switch", parse_Switch);
    registerType("!ConstantMap", parse_ConstantMap);
    registerType("!PolynomialMap", parse_PolynomialMap);
    registerType("!Any", parse_Any);
    registerType("!IdentityMap", parse_Any);
    registerType("!GroupFilter", parse_GroupFilter);
    registerType("!AxisAlignedCuboidalDomainFilter", parse_AxisAlignedCuboidalDomainFilter);
    registerType("!SphericalDomainFilter", parse_SphericalDomainFilter);
    registerType("!AffineMap", parse_AffineMap);
#ifdef EASI_USE_IMPALAJIT
    registerType("!FunctionMap", parse_FunctionMap);
#endif
#ifdef EASI_USE_LUA
    registerType("!LuaMap", parse_LuaMap);
#endif
    registerType("!SCECFile", parse_SCECFile);
#ifdef EASI_USE_ASAGI
    registerType("!ASAGI", parse_ASAGI);
#endif
    registerType("!LayeredModel", create_LayeredModel);
    registerType("!Include", create_Include);
    registerType("!EvalModel", parse_EvalModel);

    if (!m_externalAsagiReader) {
        m_asagiReader = new AsagiReader;
    }

    // Specials
    registerType("!STRESS_STR_DIP_SLIP_AM", create_Special<STRESS_STR_DIP_SLIP_AM>);
    registerType("!OptimalStress", create_Special<OptimalStress>);
    registerType("!AndersonianStress", create_Special<AndersonianStress>);
}

YAMLParser::~YAMLParser() {
    if (!m_externalAsagiReader) {
        delete m_asagiReader;
    }
}

void YAMLParser::registerType(std::string const& tag, std::function<CreateFunction> create) {
    m_creators[tag] = std::move(create);
}

Component* YAMLParser::parse(std::string const& fileName) {
    Component* root;
    try {
        const auto lastFileName = m_currentFileName;
        const auto lastPath = fs::path(lastFileName);
        const auto nextPath = fs::path(fileName);
        const auto loadFileName = [&]() {
            if (nextPath.is_relative()) {
                // remove file
                return lastPath.parent_path() / nextPath;
            }
            else {
                return nextPath;
            }
        }();
        m_currentFileName = loadFileName;
        YAML::Node config = YAML::LoadFile(loadFileName);
        root = parse(config, m_in);
        m_currentFileName = lastFileName;
    } catch (YAML::Exception const& e) {
        std::cerr << fileName << ": " << e.what() << std::endl;
        throw std::runtime_error("Error while parsing easi model file.");
    }

    return root;
}

Component* YAMLParser::parse(YAML::Node const& node, std::set<std::string> const& in) {
    auto creator = m_creators.find(node.Tag());

    if (creator == m_creators.end()) {
        throw std::invalid_argument("Unknown tag " + node.Tag());
    }

    Component* component = nullptr;
    try {
        component = creator->second(node, in, this);
    } catch (std::invalid_argument const& e) {
        std::stringstream ss;
        ss << e.what() << std::endl << node;
        throw YAML::Exception(node.Mark(), ss.str());
    }
    return component;
}

} // namespace easi

