Components
==========

In the following is a list of all components currently available in
easi.

Every component has a domain m and a codomain n which can be thought
of as a function :math:`f:\mathbb{R}^m \rightarrow \mathbb{R}^n`.
That is, a component accepts vectors in :math:`\mathbb{R}^m` and
passes vectors in :math:`\mathbb{R}^n` to its child components
(or as a result).
The dimensions are labeled and a childs input dimensions must match
its parent's output dimensions.

Composite components
--------------------

Each composite may have a sequence of child components. Composite itself
is abstract and may not be instantiated.
Maps and Filters are always composite, builders are not.

.. code-block:: YAML

    !ABSTRACT
    components:
      - <Component>
      - <Component>
      - ...

`example <https://github.com/SeisSol/easi/blob/master/examples/1_groups.yaml>`_

Alternative for composites with a single child:

.. code-block:: YAML

    !ABSTRACT
    components: <Component>

Remark: Composites must of at least one child component.
