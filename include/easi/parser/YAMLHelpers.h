/**
 * @file
 * This file is part of SeisSol.
 *
 * @author Carsten Uphoff (c.uphoff AT tum.de, http://www5.in.tum.de/wiki/index.php/Carsten_Uphoff,_M.Sc.)
 *
 * @section LICENSE
 * Copyright (c) 2017, SeisSol Group
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @section DESCRIPTION
 **/
#ifndef EASI_PARSER_YAMLHELPERS_H_
#define EASI_PARSER_YAMLHELPERS_H_

#include <set>
#include <sstream>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/type.h>
#include "easi/Component.h"

namespace easi {
void checkExistence(YAML::Node const& node, std::string const& name) {
  std::stringstream ss;
  if (node.IsScalar() || !node[name]) {
    ss << node.Tag() << ": Parameter '" << name << "' is missing." << std::endl << node;
    throw YAML::Exception(node.Mark(), ss.str());
  }
}

void checkType(YAML::Node const& node, std::string const& name, std::set<YAML::NodeType::value> const& types, bool required = true) {
  std::stringstream ss;
  if (required) {
    checkExistence(node, name);
  }
  if (node[name] && types.find(node[name].Type()) == types.end()) {
    ss << node.Tag() << ": Parameter '" << name << "' has wrong type." << std::endl << node;    
    throw YAML::Exception(node.Mark(), ss.str());
  }
}

template<typename T>
T* upcast(YAML::Node const& node, Component* component) {
  T* up = dynamic_cast<T*>(component);
  if (up == nullptr) {
    std::stringstream ss;
    ss << node.Tag() << ": Failed upcast.";
    throw YAML::Exception(node.Mark(), ss.str());
  }
  return up;
}
}

namespace YAML {
template<typename T>
struct convert<easi::Vector<T>> {
  static bool decode(Node const& node, easi::Vector<T>& rhs) {
    if(!node.IsSequence()) {
      return false;
    }
    rhs.reallocate(node.size());

    for (std::size_t i = 0; i < node.size(); ++i) {
      rhs(i) = node[i].as<T>();
    }
    return true;
  }
};

template<typename T>
struct convert<easi::Matrix<T>> {
  static bool decode(Node const& node, easi::Matrix<T>& rhs) {
    std::size_t rows = node.size();
    if(!node.IsSequence() || rows == 0) {
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
        rhs(i,j) = node[i][j].as<T>();
      }
    }
    return true;
  }
};
}

#endif
