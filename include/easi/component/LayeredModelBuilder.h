#ifndef EASI_COMPONENT_LAYEREDMODELBUILDER_H_
#define EASI_COMPONENT_LAYEREDMODELBUILDER_H_

#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace easi {

class Component;
class Map;

class LayeredModelBuilder {
public:
    enum InterpolationType { Lower, Upper, Linear };
    using Nodes = std::map<double, std::vector<double>>;
    using Parameters = std::vector<std::string>;

    inline void setMap(Map* map) { m_map = map; }
    void setInterpolationType(std::string const& interpolationType);
    inline void setInterpolationType(enum InterpolationType interpolationType) {
        m_interpolationType = interpolationType;
    }
    void setNodes(Nodes const& nodes);
    inline void setParameters(Parameters const& parameters) { m_parameters = parameters; }

    Component* getResult();

private:
    Component* createModel(const std::optional<Nodes::iterator::value_type>& lower,
                           const std::optional<Nodes::iterator::value_type>& upper,
                           std::set<std::string> const& in);

    Map* m_map = nullptr;
    enum InterpolationType m_interpolationType;
    Nodes m_nodes;
    Parameters m_parameters;
};

} // namespace easi

#endif
