#include "easi/YAMLParser.h"
#include "easi/parser/YAMLComponentParsers.inl"

easi::YAMLParser::YAMLParser(unsigned dimDomain, AsagiReader* externalAsagiReader, char firstVariable)
  : m_asagiReader(externalAsagiReader), m_externalAsagiReader(externalAsagiReader != nullptr)
{
  for (unsigned i = 0; i < dimDomain; ++i) {
    m_in.insert(std::string(1, firstVariable + i));
  }
  registerType<Switch>("!Switch");
  registerType<ConstantMap>("!ConstantMap");
  registerType<PolynomialMap>("!PolynomialMap");
  registerType<Any>("!Any");
  registerType<Any>("!IdentityMap");
  registerType<GroupFilter>("!GroupFilter");
  registerType<AxisAlignedCuboidalDomainFilter>("!AxisAlignedCuboidalDomainFilter");
  registerType<SphericalDomainFilter>("!SphericalDomainFilter");
  registerType<AffineMap>("!AffineMap");
  registerType<FunctionMap>("!FunctionMap");
  registerType<SCECFile>("!SCECFile");
#ifdef USE_ASAGI
  registerType<ASAGI>("!ASAGI");
#endif
  registerType<LayeredModelBuilder>("!LayeredModel");
  registerType<Include>("!Include");
  registerType<EvalModel>("!EvalModel");
  
  if (!m_externalAsagiReader) {
    m_asagiReader = new AsagiReader;
  }
  
  // Specials
  registerSpecial<STRESS_STR_DIP_SLIP_AM>("!STRESS_STR_DIP_SLIP_AM");
  registerSpecial<OptimalStress>("!OptimalStress");
  registerSpecial<AndersonianStress>("!AndersonianStress");
}

easi::Component* easi::YAMLParser::parse(std::string const& fileName) {
  Component* root;
  try {
    std::string lastFileName = m_currentFileName;
    m_currentFileName = fileName;
    YAML::Node config = YAML::LoadFile(fileName);
    root = parse(config, m_in);
    m_currentFileName = lastFileName;
  } catch (YAML::Exception const& e) {
    std::cerr << fileName << ": " << e.what() << std::endl;
    throw std::runtime_error("Error while parsing easi model file.");
  }

  return root;
}

easi::Component* easi::YAMLParser::parse(YAML::Node const& node, std::set<std::string> const& in) {
  auto creator = m_creators.find(node.Tag());
  
  if (creator == m_creators.end()) {
    throw std::invalid_argument("Unknown tag " + node.Tag());
  }
  
  Component* component = nullptr;
  try {
    component = (*creator->second)(node, in, this);
  } catch (std::invalid_argument const& e) {
    std::stringstream ss;
    ss << e.what() << std::endl << node;
    throw YAML::Exception(node.Mark(), ss.str());
  }
  return component;
}
