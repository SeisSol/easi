#ifndef EASI_PARSER_YAMLHELPERS_H_
#define EASI_PARSER_YAMLHELPERS_H_

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <cstddef>
#include <set>
#include <sstream>
#include <string>

namespace YAML {
template <typename T> struct convert;
}

namespace easi {

class Component;
template <typename T> class Matrix;
template <typename T> class Vector;

void checkExistence(YAML::Node const& node, std::string const& name);
void checkType(YAML::Node const& node, std::string const& name,
               std::set<YAML::NodeType::value> const& types, bool required = true);

template <typename T> T* upcast(YAML::Node const& node, Component* component) {
    T* up = dynamic_cast<T*>(component);
    if (up == nullptr) {
        std::stringstream ss;
        ss << node.Tag() << ": Failed upcast.";
        throw YAML::Exception(node.Mark(), ss.str());
    }
    return up;
}

} // namespace easi

namespace YAML {

template <typename T> struct convert<easi::Vector<T>> {
    static bool decode(Node const& node, easi::Vector<T>& rhs) {
        if (!node.IsSequence()) {
            return false;
        }
        rhs.reallocate(node.size());

        for (std::size_t i = 0; i < node.size(); ++i) {
            rhs(i) = node[i].as<T>();
        }
        return true;
    }
};

template <typename T> struct convert<easi::Matrix<T>> {
    static bool decode(Node const& node, easi::Matrix<T>& rhs) {
        std::size_t rows = node.size();
        if (!node.IsSequence() || rows == 0) {
            return false;
        }
        std::size_t maxCols = 0;
        for (std::size_t i = 0; i < node.size(); ++i) {
            if (!node.IsSequence()) {
                return false;
            }
            maxCols = std::max(maxCols, node[i].size());
        }
        rhs.reallocate(rows, maxCols);

        for (std::size_t i = 0; i < node.size(); ++i) {
            for (std::size_t j = 0; j < node[i].size(); ++j) {
                rhs(i, j) = node[i][j].as<T>();
            }
        }
        return true;
    }
};

} // namespace YAML

#endif
