Getting Started with easi
=========================

easi is a library written in C++14.
It needs to be compiled with Cmake.

Dependencies
------------

Easi depends on the following three projects:

-  `yaml-cpp <https://github.com/jbeder/yaml-cpp>`__
-  `ASAGI <https://github.com/TUM-I5/ASAGI/>`__
-  `ImpalaJIT <https://github.com/uphoffc/ImpalaJIT>`__

Only yaml-cpp (version 0.6.x) is a required dependency.
It can be obtained, for example, from the package repositories of most Linux distributions or as a module on SuperMUC-NG.

Compilation
------------
Before installing easi, you first need to install all dependencies.
Next, clone the easi repository and create a separate build directory.
The directory can be outside of the repository.
This makes the compilation cleaner.
Finally, execute the following lines:

.. code-block:: bash
  # $EASI_SRC is the directory into which you've cloned the easi source
  cmake -DCMAKE_PREFIX_PATH=$HOME -DCMAKE_INSTALL_PREFIX=$HOME -DASAGI=ON -DIMPALAJIT=ON $EASI_SRC
  make -j4 install

This installs easi into your home directory.

Usage example
-------------

easi is configured via `YAML <http://yaml.org>`__ configuration files.
For example, such a configuration file could look like the following:

.. code-block:: yaml

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

Here, all points with y-coordinate inbetween -5 km and +5 km would be
assigned constant model parameters. For all other points, a linear
interpolation, depending on the z-coordinate is used.

Application example
-------------------

The first step is always to create a model.
Here, we may use the YAMLParser class which creates models from YAML configuration files.

.. code-block:: cpp
  :linenos:

  easi::YAMLParser parser(3);
  easi::Component* model = parser.parse("test.yaml");  

The argument in YAMLParser's constructor is the dimension of the input vectors.
Here, we take 3 as we want to query our model in a 3-dimensional space.

As a next step, we need to define a query, which defines the input vectors
for which we want to evaluate our model.
In the following example we add the points (1,2,3) and (2,3,4).
Each point may have an additional group parameter, which may be used
to distinguish points in an easi file.

.. code-block:: cpp

  easi::Query query(2,3);
  query.x(0,0) = 1.0;
  query.x(0,1) = 2.0;
  query.x(0,2) = 3.0;
  query.group(0) = 1;
  query.x(1,0) = 2.0;
  query.x(1,1) = 3.0;
  query.x(1,2) = -4.0;
  query.group(1) = 1;

We need to store the output vectors somewhere.
For this purpose, we always need to supply an adapter, which connects
the output vector with locations in memory.
In our sample application, the output vector shall be stored as array of
structs, and hence we use an ArrayOfStructsAdapter.
(Note that additional adapters can be implemented by overriding the class ResultAdapter.)

.. code-block:: cpp
  
  struct ElasticMaterial {
    double lambda, mu, rho;
  };

  ElasticMaterial material[2];
  easi::ArrayOfStructsAdapter<ElasticMaterial> adapter(material);
  adapter.addBindingPoint("lambda", &ElasticMaterial::lambda);
  adapter.addBindingPoint("mu",     &ElasticMaterial::mu);
  adapter.addBindingPoint("rho",    &ElasticMaterial::rho);

Finally, a simple call to evaluate is sufficient, and the model should be
deleted if is not required anymore.

.. code-block:: cpp

    model->evaluate(query, adapter);
    delete model;

The whole sample code is listed in the following:

.. code-block:: cpp

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
