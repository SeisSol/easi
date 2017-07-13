# easi

easi is a library for the Easy Initialization of model parameters in three (or less) dimensional domains.
The library is designed for use in grid-based simulation software, such as [SeisSol](https://github.com/SeisSol/SeisSol).

The main idea is to describe a parameter model as a tree, where each inner node of the tree may have one
of the following purposes:

| Node type     | Purpose |
| ------------- | ------- |
| filter        | Accept only points with certain properties. |
| map           | Map m-dimensional points to n-dimensional points. |

Each node has labeled input dimensions and labeled output dimensions. E.g. a map might take x,y,z as input and return &rho;, &mu;, and &lambda;. When arriving at the leaf of the tree, the requested dimensions are set according to the provided dimensions. E.g. if an application ask for &rho; and &mu;, these values are set according to the map.
A map might also be just used for intermediate computation. E.g. assume a map computes &rho;, &mu;, and &lambda; from the spatial dimensions x, y, and z. This map may have children which compute Qp and Qs but depending on &rho;, &mu;, and &lambda;.

## Dependencies and build instructions
easi itself is a header-only library. (Only unit tests must be compiled with CMake.) It depends on

* [ImpalaJIT](https://github.com/Manuel1605/ImpalaJIT)
* [yaml-cpp](https://github.com/jbeder/yaml-cpp)

Make sure to link against these libraries when using easi.

easi requires C++11.

## Documentation
The reference documentation can be found in the [wiki](https://github.com/SeisSol/easi/wiki).

## Example
easi is configured via [YAML](http://yaml.org) configuration files. For example, such a configuration file
could look like the following:
```YAML
!Any
components:
  - !AxisAlignedCuboidalDomainFilter
    limits:
      x: [-100000, 100000]
      y: [-5000, 5000]
      z: [-100000, 10000]
    components:
      - !ConstantMap
        map:
          lambda: 1e10
          mu: 2e10
          rho: 5000
  - !LayeredModel
    map: !AffineMap
      matrix:
        z: [0, 0, 1]
      translation:
        z: 0
    interpolation: linear
    parameters: [rho, mu, lambda]
    nodes:
      -100.0:   [2300.0, 0.1766e10, 0.4999e10]
      -300.0:   [2300.0, 0.6936e10, 1.3872e10]
      -1000.0:  [2600.0, 1.3717e10, 1.8962e10]
      -3000.0:  [2700.0, 2.1168e10, 2.7891e10]
      -6000.0:  [2870.0, 3.1041e10, 3.8591e10]
      -31000.0: [3500.0, 3.9847e10, 4.3525e10]
      -50000.0: [3200.0, 6.4800e10, 6.5088e10] 
```
Here, all points with y-coordinate inbetween -5 km and +5 km would be assigned constant model parameters.
For all other points, a linear interpolation, depending on the z-coordinate is used.

In order to use this model inside an application, the following code is sufficient:

```c++
#include <iostream>
#include "easi/YAMLParser.h"
#include "easi/ResultAdapter.h"

struct ElasticMaterial {
  double lambda, mu, rho;
};

int main(int argc, char** argv)
{
  easi::Query query(2,3);
  query.x(0,0) = 1.0;
  query.x(0,1) = 2.0;
  query.x(0,2) = 3.0;
  query.group(0) = 1;
  query.x(1,0) = 2.0;
  query.x(1,1) = 3.0;
  query.x(1,2) = -4.0;
  query.group(1) = 1;
  
  easi::YAMLParser parser(3);
  easi::Component* model = parser.parse("test.yaml");  
  
  ElasticMaterial material[2];
  easi::ArrayOfStructsAdapter<ElasticMaterial> adapter(material);
  adapter.addBindingPoint("lambda", &ElasticMaterial::lambda);
  adapter.addBindingPoint("mu",     &ElasticMaterial::mu);
  adapter.addBindingPoint("rho",    &ElasticMaterial::rho);
  
  model->evaluate(query, adapter);
  
  delete model;
  
  for (unsigned j = 0; j < 2; ++j) {
    std::cout << material[j].lambda << " " << material[j].mu << " " << material[j].rho << std::endl;
  }  

  return 0;
}

```
