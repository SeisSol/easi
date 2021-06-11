Maps
====

A map allows to map vectors from :math:`\mathbb{R}^m` to :math:`\mathbb{R}^n`.

ConstantMap
-----------

Assigns a constant value, independent of position.

.. code-block:: YAML

    !ConstantMap
    map:
      <dimension>: <double>
      <dimension>: <double>
    ...

:Domain:
  *inherited*
:Codomain:
  keys in map
:Example:
  `0_constant <https://github.com/SeisSol/easi/blob/master/examples/0_constant.yaml>`_

IdentityMap
-----------

Does nothing in particular (same as !Any).

.. code-block:: YAML

    !IdentityMap

:Domain:
  *inherited*
:Codomain:
  *domain*

AffineMap
---------

Implements the affine mapping :math:`y=Ax+t`.

.. code-block:: YAML

    !AffineMap
    matrix:
      <dimension>: [<double>, <double>, ...]
      <dimension>: [<double>, <double>, ...]
      ...
    translation:
      <dimension>: <double>
      <dimension>: <double>
      ...

:Domain:
  *inherited*
:Codomain:
  keys of matrix / translation
:Example:
  Say we have a row in matrix which reads "p: [a,b,c]", and a
  corresponding row in translation "p: d".
  Furthermore, assume rho, mu, and lambda are the input dimensions.
  Then :math:`p = a\cdot\lambda + b\cdot\mu + c\cdot\rho + d.`

  `3_layered_linear <https://github.com/SeisSol/easi/blob/master/examples/3_layered_linear.yaml>`_


By convention, the input dimensions are ordered lexicographically
(according to the ASCII code, i.e. first 0-9, then A-Z, and then a-z),
hence the first entry in a matrix row corresponds to the the first input
dimension in lexicographical order.

PolynomialMap
-------------

Assigns a value using a polynomial for every parameter.

.. code-block:: YAML

    !PolynomialMap
    map:
      #             x^n,      ...,        x,        1
      <dimension>: [<double>, ..., <double>, <double>]
      <dimension>: [<double>, ..., <double>, <double>]
      ...

:Domain:
  *inherited* but may only be a one dimension
:Codomain:
  keys in map
:Example:
  `62_landers <https://github.com/SeisSol/easi/blob/master/examples/62_landers.yaml#L35>`_

FunctionMap
-----------

Implements a mapping described by an ImpalaJIT function.

.. code-block:: YAML

    !FunctionMap
    map:
      <dimension>: <function_body>
      <dimension>: |
        <long_function_body>
      ...

:Domain:
  *inherited*
:Codomain:
  keys in map
:Example:
  Input dimensions are x,y,z. Then "p: return x \* y \* z;"
  yields :math:`p = x \cdot y \cdot z`.
  (Note: Don't forget the return statement.)

  `5_function <https://github.com/SeisSol/easi/blob/master/examples/5_function.yaml>`_

The <function_body> must an be ImpalaJIT function (without surrounding curly braces).
The function gets passed all input dimensions automatically.

**Known limitations:**

- No comments (// or /\* \*/)
- No exponential notation (use pow(10.,3.) instead of 1e3)
- No 'else if' (use else { if () {}}).

ASAGI
-----

Looks up values using ASAGI (with trilinear interpolation).

.. code-block:: YAML

    !ASAGI
    file: <string>
    parameters: [<parameter>,<parameter>,...]
    var: <string>
    interpolation: (nearest|linear)

:Domain:
  *inherited*
:Codomain:
  keys in parameters
:Example:
  `101_asagi <https://github.com/SeisSol/easi/blob/master/examples/101_asagi.yaml>`_
:file:
  Path to a NetCDF file that is compatible with ASAGI
:parameters:
  Parameters supplied by ASAGI in order of appearance in
  the NetCDF file
:var:
  The NetCDF variable which holds the data (default: data)
:interpolation:
  Choose between nearest neighbour and linear interpolation (default: linear)

SCECFile
--------

Looks up fault parameters in SCEC stress file (as in
http://scecdata.usc.edu/cvws/download/tpv16/TPV16\_17\_Description\_v03.pdf).

.. code-block:: YAML

    !SCECFile
    file: <string>
    interpolation: (nearest|linear)

:Domain:
  *inherited*, must be 2D
:Codomain:
  cohesion, d\_c, forced\_rupture\_time, mu\_d, mu\_s, s\_dip, s\_normal, s\_strike
:Example:
  `example <https://github.com/SeisSol/easi/blob/master/examples/f_16_scec.yaml#L8>`_
:file:
  Path to a SCEC stress file
:interpolation:
  Choose between nearest neighbour and linear interpolation (default: linear)

EvalModel
---------

Provides values by evaluating another easi tree.

.. code-block:: YAML

    !EvalModel
    parameters: [<parameter>,<parameter>,...]
    model: <component>
    ... # specify easi tree
    components: <component>
    ... # components receive output of model as input

:Domain:
  *inherited*
:Codomain:
  keys of parameters
:Example:
  `120_initial_stress <https://github.com/SeisSol/easi/blob/9e93f35fbacc950d00534643c59a64dff306a381/examples/120_initial_stress.yaml#L19>`__:
  [b\_xx, b\_yy, b\_zz, b\_xy, b\_yz, b\_xz] are defined through the
  component "!STRESS\_STR\_DIP\_SLIP\_AM", which depends itself on several
  parameters (mu\_d, mu\_s, etc). One of these parameter "strike" is set
  to vary spatially through an "!AffineMap". "!EvalModel" allows to
  evaluate this intermediate variable before executing the
  "!STRESS\_STR\_DIP\_SLIP\_AM" component.

OptimalStress
-------------

This component generates a stress tensor which maximizes shear traction
on the optimally oriented plane defined by the strike and dip angles, along the rake angle orientation.
Such optimally oriented plane can be a virtual plane if it does not correspond to any segment of the fault system. 

The principal stress magnitudes are prescribed by:

- the relative prestress ratio R (where :math:`R=1/(1+S)`, with S the relative fault strength), 
- the stress shape ratio s2ratio :math:`= (s_2-s_3)/(s_1-s_3)`, where :math:`s_1`, :math:`s_2` and :math:`s_3` are the maximum, intermediate, and minimum compressive stress, respetively, and 
- either the :math:`s_{zz}` component of the stress tensor, or the mean effective stress, effectiveConfiningStress :math:`= Tr(s_{ii})/3`. (if s_zz is set, effectiveConfiningStress should be set 0, and vice versa).

To prescribe R, static and dynamic friction (mu\_s and mu\_d) as well as cohesion are required. 
The procedure is described in Ulrich et al. (2019), methods section 'Initial Stress'.


.. code-block:: YAML

        components: !OptimalStress
          constants:
            mu_d:      <double>
            mu_s:      <double>
            strike:    <double>
            dip:       <double>
            rake:      <double>
            cohesion:  <double>
            s2ratio:   <double>
            R:         <double>
            effectiveConfiningStress: <double>
            s_zz: <double>

:Domain:
  *inherited*
:Codomain:
  stress components (b\_xx, b\_yy, b\_zz, b\_xy, b\_yz, and b\_xz)

AndersonianStress
-----------------

This component allows computing Andersonian stress tensors (for which one principal axis of the stress tensor is vertical).

The principal stress orientations are defined by SH_max (measured from North, positive eastwards), the direction of maximum horizontal compressive stress.

S_v defines which of the principal stresses :math:`s_i` is vertical where :math:`s_1>s_2>s_3`.
S_v = 1, 2, or 3 should be used when the vertical principal stress is the maximum, intermediate, or minimum compressive stress, respetively.
Assuming mu_d=0.6, S_v = 1 favours normal faulting on a 60° dipping fault plane striking SH_max,
S_v = 2 favours strike-slip faulting on a vertical fault plane making an angle of 30° with SH_max and
S_v = 3 favours reverse faulting on a 30° dipping fault plane striking SH_max.

The principal stress magnitudes are prescribed by the relative fault strength S (related to the relative prestress ratio R by :math:`R=1/(1+S)`), 
the vertical stress sig_zz and the stress shape ratio
:math:`s2ratio = (s_2-s_3)/(s_1-s_3)`, where :math:`s_1>s_2>s_3` are the principal stress
magnitudes, following the procedure described in Ulrich et al.
(2019), methods section 'Initial Stress'. To prescribe S, static and dynamic friction
(mu\_s and mu\_d), as well as cohesion, are required. 


.. code-block:: YAML

        components: !AndersonianStress
          constants:
            mu_d:      <double>
            mu_s:      <double>
            SH_max:    <double>
            S_v:       <int (1,2 or 3)>
            cohesion:  <double>
            s2ratio:   <double>
            S:         <double>
            sig_zz:    <double>

:Domain:
  *inherited*
:Codomain:
  stress components (b\_xx, b\_yy, b\_zz, b\_xy, b\_yz, and b\_xz)



SpecialMap
----------

Evaluates application-defined functions.

.. code-block:: YAML

    !<registered-name>
    constants:
      <parameter>: <double>
      <parameter>: <double>
      ...

:Domain:
  *inherited* without constant parameters
:Codomain:
  user-defined
:Example:
  We want to create a function which takes three input parameters
  and supplies two output parameters:
  
  .. code-block:: cpp

    #include "easi/util/MagicStruct.h"

    struct Special {
      struct in {
        double i1, i2, i3;
      };
      in i;

      struct out {
        double o1, o2;
      };
      out o;

      inline void evaluate() {
          o.o1 = exp(i.i1) + i.i2;
          o.o2 = i.i3 * o.o1;
      }
    };

    SELF_AWARE_STRUCT(Special::in, i1, i2, i3)
    SELF_AWARE_STRUCT(Special::out, o1, o2)
  
  Register this file with the parser:

  .. code-block:: cpp

    easi::YAMLParser parser(3);
    parser.registerSpecial<Special>("!Special");

  And use it in the following way, e.g.:

  .. code-block:: yaml

    !Special
    constants:
      i2: 3.0

  The domain of !Special is now i1, i3 and the codomain is o1, o2.
  i2 is constant and has the value 3.


Deprecated components
---------------------


STRESS\_STR\_DIP\_SLIP\_AM
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: YAML

        components: !STRESS_STR_DIP_SLIP_AM
          constants:
            mu_d:      <double>
            mu_s:      <double>
            strike:    <double>
            dip:       <double>
            DipSlipFaulting: <double> (0 or 1)
            cohesion:  <double>
            s2ratio:   <double>
            s_zz:      <double>
            R:         <double>
            s2ratio:   <double>

:Domain:
  *inherited*
:Codomain:
  normalized stress components (b\_xx, b\_yy, b\_zz, b\_xy, b\_yz, and b\_xz, with b\_zz=1)
:Example:
  `120_initial_stress <https://github.com/SeisSol/easi/blob/9e93f35fbacc950d00534643c59a64dff306a381/examples/120_initial_stress.yaml#L44>`__

This component, used for example in the setup of the Sumatra SC paper 
(Uphoff et al., 2017) is deprecated and can be substituted by the more complete 'OptimalStress' routine. 
While in the 'OptimalStress' routine, a rake parameter defines the direction of maximized shear traction 
on the optimally oriented fault, such direction is here defined by the parameter
DipSlipFaulting (1 for pure dip-slip, 0 for pure strike-slip).
Another difference with the OptimalStress component is that STRESS\_STR\_DIP\_SLIP\_AM returns a normalized stress tensor.

The following code:

.. code-block:: YAML

    !STRESS_STR_DIP_SLIP_AM
        constants:
            mu_d:     a
            mu_s:     b
            strike:   c
            dip:      d
            DipSlipFaulting:  1.0
            cohesion: e
            s2ratio:  f
            R:        g
            s_zz:     h

is equivalent to:

.. code-block:: YAML

    !EvalModel
    parameters: [b_xx,b_yy,b_zz,b_xy, b_xz, b_yz]
    model: !ConstantMap
        map:
                mu_d: a
        components: !OptimalStress
              constants:
                mu_s:      b
                strike:    c
                dip:       d
                rake:     90.0
                cohesion:  e
                s2ratio:   f
                R:         g
                s_zz:      h
                effectiveConfiningStress: 0.0
    components: !FunctionMap
        map:
           b_xx: return b_xx/b_zz;
           b_yy: return b_yy/b_zz;
           b_zz: return 1;
           b_xy: return b_xy/b_zz;
           b_xz: return b_xz/b_zz;
           b_yz: return b_yz/b_zz;
