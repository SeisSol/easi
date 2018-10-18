Builders
========

Builders are not components by itself, but build a subtree using
available components.

LayeredModel
------------

Defines parameters at nodes, which are interpolated inbetween nodes.

.. code-block:: YAML

    !LayeredModel
    map: <map> # Mapping to 1D (root component)
    interpolation: (lower|upper|linear) 
    parameters: [<dimension>, <dimension>, ...] # order of dimension
    nodes:
      <double>: [<double>, <double>, ...] # key: node position, value: dimension values
      <double>: [<double>, <double>, ...]
      ...

:Domain:
  *inherited*, must be 1D
:Codomain:
  length of coefficients sequence
:Interpolation methods:
  :lower:
    Take the value of the lower (smaller) node
  :upper:
    Take the value of the upper (larger) node
  :linear:
    Linear interpolation
:Example:
  `3_layered_linear <https://github.com/SeisSol/easi/blob/master/examples/3_layered_linear.yaml>`_

Include
-------

Includes another configuration file.

.. code-block:: YAML

    !Include <filename>

:Example:
  `f_120_sumatra <https://github.com/SeisSol/easi/blob/master/examples/f_120_sumatra.yaml#L24>`_
