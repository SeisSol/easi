Filters
=======

Filters accept only a subsets of points and allows for the spatial
partitioning of models.

Any
---

Any mostly serves as a root node and accepts every point and every
group.

.. code-block:: YAML

    !Any

:Domain:
  *inherited*
:Codomain:
  *domain*
:Example:
  `1_groups <https://github.com/SeisSol/easi/blob/3f5783097808c486962fe8fa253f7738db3cfacb/examples/1_groups.yaml#L1>`_

AxisAlignedCuboidalDomainFilter
-------------------------------

Accepts only points inside an axis-aligned bounding box, i.e. when
:math:`l_x \leq x \leq u_x` and :math:`l_y \leq y \leq u_y` and ...

.. code-block:: YAML

    !AxisAlignedCuboidalDomainFilter
    limits:
      <dimension>: [<double>, <double>] # [l_x, u_x]
      <dimension>: [<double>, <double>] # [l_y, u_y]
      ...

:Domain:
  *codomain*
:Codomain:
  keys of limits
:Example:
  `f_103_scec <https://github.com/SeisSol/easi/blob/master/examples/f_103_scec.yaml#L28>`_

SphericalDomainFilter
---------------------

Accepts only points inside a sphere, i.e. when :math:`\lVert x-c\rVert \leq r`.

.. code-block:: YAML

    !SphericalDomainFilter
    radius: <double>
    centre:
      <dimension>: <double>
      <dimension>: <double>
      ...

:Domain:
  *codomain*
:Codomain:
  keys of centre

GroupFilter
-----------

Accepts only points belonging to a set of groups.

.. code-block:: YAML

    !GroupFilter
    groups: [<int>,<int>,...]

:Domain:
  *inherited*
:Codomain:
  *domain*
:Example:
  `120_sumatra <https://github.com/SeisSol/easi/blob/master/examples/120_sumatra.yaml#L7>`__

Switch
------

Can be used to use select a component based on the requested parameters.

.. code-block:: YAML

    !Switch
    [<parameter>,<parameter>,...]: <component>
    [<parameter>,<parameter>,...]: <component>
    ...

:Domain:
  *inherited*
:Codomain:
  *domain*
:Example:
  `120_sumatra <https://github.com/SeisSol/easi/blob/master/examples/120_sumatra.yaml#L1>`__:
  [mu\_d, mu\_s, d\_c] are defined with a !ConstantMap
  and [cohesion, forced\_rupture\_time] are defined with a !FunctionMap.

The component on the right-hand side of the colon specifies a sub-model for the parameters on the
left-hand side of the colon.
The parameter lists must not intersect, as otherwise the sub-model, which shall be evaluated for
a parameter, would not be uniquely defined.
