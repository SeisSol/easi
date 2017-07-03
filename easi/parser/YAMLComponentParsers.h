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
#ifndef EASI_PARSER_YAMLCOMPONENTPARSERS_H_
#define EASI_PARSER_YAMLCOMPONENTPARSERS_H_

#include "easi/parser/YAMLHelpers.h"
#include "easi/component/Switch.h"
#include "easi/component/IdentityMap.h"
#include "easi/component/ConstantModel.h"
#include "easi/component/FunctionModel.h"
#include "easi/component/PolynomialModel.h"
#include "easi/component/Any.h"
#include "easi/component/DomainFilter.h"
#include "easi/component/GroupFilter.h"
#include "easi/component/AffineMap.h"
#include "easi/component/FunctionMap.h"
#include "easi/component/LayeredModelBuilder.h"

namespace easi {
template<typename T>
void parse(T* component, YAML::Node const&, unsigned, YAMLAbstractParser*) {
}

template<>
void parse<Model>(Model* component, YAML::Node const& node, unsigned dimDomain, YAMLAbstractParser* parser) {
  parse<Component>(component, node, dimDomain, parser);
}

template<>
void parse<ConstantModel>(ConstantModel* component, YAML::Node const& node, unsigned dimDomain, YAMLAbstractParser* parser) {
  checkType(node, "parameters", {YAML::NodeType::Map});
  
  ConstantModel::Parameters parameters = node["parameters"].as<ConstantModel::Parameters>();
  
  component->setDimDomain(dimDomain);
  component->setParameters(parameters);
  parse<Model>(component, node, dimDomain, parser);
}

template<>
void parse<FunctionModel>(FunctionModel* component, YAML::Node const& node, unsigned dimDomain, YAMLAbstractParser* parser) {
  checkType(node, "parameters", {YAML::NodeType::Map});
  
  FunctionModel::Parameters parameters = node["parameters"].as<FunctionModel::Parameters>();
  
  component->setParameters(parameters);
  parse<Model>(component, node, dimDomain, parser);
}

template<>
void parse<PolynomialModel>(PolynomialModel* component, YAML::Node const& node, unsigned dimDomain, YAMLAbstractParser* parser) {
  checkType(node, "coefficients", {YAML::NodeType::Sequence});
  checkType(node, "parameters", {YAML::NodeType::Sequence});
  
  Matrix<double> coeffs = node["coefficients"].as<Matrix<double>>();
  component->setCoefficients(coeffs);
  PolynomialModel::Parameters parameters = node["parameters"].as<PolynomialModel::Parameters>();
  component->setParameters(parameters);

  parse<Model>(component, node, dimDomain, parser);
}

template<>
void parse<Composite>(Composite* component, YAML::Node const& node, unsigned dimDomain, YAMLAbstractParser* parser) {
  checkType(node, "components", {YAML::NodeType::Sequence}, false);

  if (node["components"]) {
    for (YAML::const_iterator it = node["components"].begin(); it != node["components"].end(); ++it) {
      Component* child = parser->parse(*it, component->dimCodomain());
      component->add(child);
    }
  }
  parse<Component>(component, node, dimDomain, parser);
}

template<>
void parse<Switch>(Switch* component, YAML::Node const& node, unsigned dimDomain, YAMLAbstractParser* parser) {
  checkType(node, "components", {YAML::NodeType::Sequence}, false);

  component->setDimension(dimDomain);
  for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
    Component* child = parser->parse(it->second, component->dimCodomain());
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
  parse<Component>(component, node, dimDomain, parser);
}

template<>
void parse<Filter>(Filter* component, YAML::Node const& node, unsigned dimDomain, YAMLAbstractParser* parser) {
  component->setDimension(dimDomain);
  parse<Composite>(component, node, dimDomain, parser);
}

template<>
void parse<Any>(Any* component, YAML::Node const& node, unsigned dimDomain, YAMLAbstractParser* parser) {
  parse<Filter>(component, node, dimDomain, parser);
}

template<>
void parse<GroupFilter>(GroupFilter* component, YAML::Node const& node, unsigned dimDomain, YAMLAbstractParser* parser) {
  checkType(node, "groups", {YAML::NodeType::Scalar, YAML::NodeType::Sequence});
  
  std::set<int> groups;
  if (node["groups"].IsScalar()) {
    groups.insert(node["groups"].as<int>());
  } else {
    std::vector<int> groupSeq = node["groups"].as<std::vector<int>>();
    groups.insert(groupSeq.begin(), groupSeq.end());
  }
  
  component->setDimension(dimDomain);
  component->setGroups(groups);
  parse<Filter>(component, node, dimDomain, parser);
}

template<>
void parse<AxisAlignedCuboidalDomainFilter>(AxisAlignedCuboidalDomainFilter* component, YAML::Node const& node, unsigned dimDomain, YAMLAbstractParser* parser) {
  checkType(node, "limits", {YAML::NodeType::Map});
  
  auto limitMap = node["limits"].as< std::map<std::string, std::pair<double, double>> >();
  
  std::vector<std::pair<double, double>> limits;
  for (auto it = limitMap.cbegin(); it != limitMap.cend(); ++it) {
    limits.push_back(it->second);
  }
  
  component->setDomain(limits);
  checkDomain(node, component, dimDomain);
  
  parse<Filter>(component, node, dimDomain, parser);
}

template<>
void parse<SphericalDomainFilter>(SphericalDomainFilter* component, YAML::Node const& node, unsigned dimDomain, YAMLAbstractParser* parser) {
  checkType(node, "radius", {YAML::NodeType::Scalar});
  checkType(node, "centre", {YAML::NodeType::Sequence});
  
  double radius = node["radius"].as<double>();
  Vector<double> centre = node["centre"].as<Vector<double>>();
  
  component->setDomain(radius, centre);
  checkDomain(node, component, dimDomain);

  parse<Filter>(component, node, dimDomain, parser);
}


template<>
void parse<Map>(Map* component, YAML::Node const& node, unsigned dimDomain, YAMLAbstractParser* parser) {
  parse<Composite>(component, node, dimDomain, parser);
}

template<>
void parse<IdentityMap>(IdentityMap* component, YAML::Node const& node, unsigned dimDomain, YAMLAbstractParser* parser) {
  component->setDimension(dimDomain);
  parse<Map>(component, node, dimDomain, parser);
}

template<>
void parse<AffineMap>(AffineMap* component, YAML::Node const& node, unsigned dimDomain, YAMLAbstractParser* parser) {
  checkType(node, "matrix", {YAML::NodeType::Sequence});
  checkType(node, "translation", {YAML::NodeType::Sequence});
  
  Matrix<double> matrix = node["matrix"].as<Matrix<double>>();
  Vector<double> translation = node["translation"].as<Vector<double>>();

  component->setMap(matrix, translation);
  checkDomain(node, component, dimDomain);

  parse<Map>(component, node, dimDomain, parser);
}

template<>
void parse<FunctionMap>(FunctionMap* component, YAML::Node const& node, unsigned dimDomain, YAMLAbstractParser* parser) {
  checkType(node, "map", {YAML::NodeType::Map});
  
  FunctionMap::Parameters parameters = node["map"].as<FunctionMap::Parameters>();
  
  component->setMap(parameters);
  parse<Map>(component, node, dimDomain, parser);
}

template<typename T>
Component* create(YAML::Node const& node, unsigned dimDomain, YAMLAbstractParser* parser) {
  T* component = new T;
  parse<T>(component, node, dimDomain, parser);
  return component;
}

template<>
Component* create<LayeredModelBuilder>(YAML::Node const& node, unsigned dimDomain, YAMLAbstractParser* parser) {
  LayeredModelBuilder builder;
  
  checkExistence(node, "map");
  checkType(node, "interpolation", {YAML::NodeType::Scalar});
  checkType(node, "nodes", {YAML::NodeType::Map});
  checkType(node, "parameters", {YAML::NodeType::Sequence});
  
  Component* rawMap = parser->parse(node["map"], dimDomain);
  Map* map = upcast<Map>(node, rawMap);
  std::string interpolation = node["interpolation"].as<std::string>();
  LayeredModelBuilder::Nodes nodes = node["nodes"].as<LayeredModelBuilder::Nodes>();
  PolynomialModel::Parameters parameters = node["parameters"].as<PolynomialModel::Parameters>();
  
  builder.setMap(map);
  builder.setInterpolationType(interpolation);
  builder.setNodes(nodes);
  builder.setParameters(parameters);
  
  return builder.getResult();
}
}

#endif
