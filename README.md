# easi

easi is a library for the **E**asy **I**nitialization of models in three (or less or more) dimensional domains.
The purpose of easi is to evaluate functions f : R^m -> R^n,
which are described in a [YAML](http://yaml.org) configuration file.
In grid-based simulation software, such as [SeisSol](http://www.seissol.org>),
easi may be used to define models.
In SeisSol, the function f maps every point in space (say x,y,z) to a vector of parameters (e.g. ρ, μ, and λ),
which define a rheological model.
Here, over 5000 lines of model-specific Fortran code could be replaced with YAML files.

## Documentation
The reference documentation can be found on [Read the Docs](https://easyinit.readthedocs.io) or be downloaded as [PDF manual](http://readthedocs.org/projects/easyinit/downloads/pdf/latest/).
