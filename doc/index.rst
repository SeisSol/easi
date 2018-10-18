.. easi documentation master file, created by
   sphinx-quickstart on Wed Oct 17 17:17:37 2018.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

====
easi
====

easi is a library for the **E**\asy **I**\nitialization of models in three (or less or more) dimensional domains.
The purpose of easi is to evaluate functions :math:`f:\mathbb{R}^m\rightarrow \mathbb{R}^n`,
which are described in a `YAML <http://yaml.org>`_ configuration file.
In grid-based simulation software, such as `SeisSol <http://www.seissol.org>`_,
easi may be used to define models.
In SeisSol, the function f maps every point in space (say x,y,z) to a vector of parameters (e.g. ρ, μ, and λ),
which define a rheological model.
Here, over 5000 lines of model-specific Fortran code could be replaced with YAML files.

An easi model consists of only two components: Map and Filter.
These components may be wired as a tree, e.g. as in the following figure:

.. image:: figs/tree.png
    :width: 50%
    :align: center

The procedure is as follows:
A point x,y,z enters the tree at the root.
The first Map takes a 3-dimensional vector as input and returns a n-dimensional vector.
The following Filters decide if they accept this n-dimensional vector or reject it (e.g. accept if it lies in a hypercube and reject otherwise).
Then, the branch which accepts a vector is taken (a Map accepts everything by default).
The final parameter vector is, in this example, always 3-dimensional as it contains the density and the Lamé parameters.

.. toctree::
  :maxdepth: 2
  :caption: Contents

  getting_started
  components
  maps
  filters
  builders
  glossary
