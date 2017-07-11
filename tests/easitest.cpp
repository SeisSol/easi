#include "easitest.h"

#include <iostream>
#include <easi/YAMLParser.h>
#include <easi/ResultAdapter.h>

bool equal(double ref, double value) {
  bool isEqual = fabs(ref-value) <= std::numeric_limits<double>::epsilon();
  if (!isEqual) {
    std::cerr << "Expected: " << ref << "; actual: " << value << "; error: " << fabs(ref-value) << std::endl;
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
