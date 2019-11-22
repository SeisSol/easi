#include "easitest.h"

#include <cmath>
#include <iostream>
#include <iomanip>
#include <easi/YAMLParser.h>
#include <easi/ResultAdapter.h>

bool equal(double value, double ref, double eps) {
  bool isEqual = fabs(ref-value) <= eps;
  if (!isEqual) {
    std::cerr << std::setprecision(16) << "Expected: " << ref << "; actual: " << value << "; error: " << fabs(ref-value) << std::endl;
  }
  return isEqual;
}

std::vector<ElasticMaterial> elasticModel(std::string const& fileName, easi::Query& query) {
  easi::YAMLParser parser(query.dimDomain());
  easi::Component* model = parser.parse(fileName);
  
  std::vector<ElasticMaterial> material(query.numPoints());
  easi::ArrayOfStructsAdapter<ElasticMaterial> adapter(material.data());
  adapter.addBindingPoint("lambda", &ElasticMaterial::lambda);
  adapter.addBindingPoint("mu",     &ElasticMaterial::mu);
  adapter.addBindingPoint("rho",    &ElasticMaterial::rho);
  
  model->evaluate(query, adapter);  
  delete model;
  
  return material;
}

std::vector<std::vector<double>> genericModel(std::string const& fileName, easi::Query& query, std::vector<std::string> const& parameters) {
  easi::YAMLParser parser(query.dimDomain());
  easi::Component* model = parser.parse(fileName);
  
  easi::ArraysAdapter<> adapter;
  std::vector<std::vector<double>> material(parameters.size());
  auto it = material.begin();
  for (auto const& p : parameters) {
    it->resize(query.numPoints());
    adapter.addBindingPoint(p, it->data());
    ++it;
  }
  
  model->evaluate(query, adapter);  
  delete model;
  
  return material;
}

