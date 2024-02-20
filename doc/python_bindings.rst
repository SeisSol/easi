Python wrapper
===============

Alternatively to using easi directly via cpp code, easi can be used through python wrappers.

Installation
------------

The python wrappers have to first be installed with the `-DPYTHON_BINDINGS=ON` cmake option.
Note that ASAGI and ImpalaJIT have to be compiled with `cmake .. -DCMAKE_CXX_FLAGS="-fPIC"`.
The easi python module can then be found by python when the path to the library (e.g. easi.cpython-310-x86_64-linux-gnu.so) is added to the $PYTHONPATH environment variable.

```bash
export PYTHONPATH=your_build_folder/python_bindings:$PYTHONPATH
```

Use
---

The module can then be used as follows:

```python
import numpy as np
import easi

coords = np.random.rand(10,3)
region = np.random.randint(1,3, 10)
print(easi.evaluate_model(coords, region, ["rho", "lambda"], "test.yaml"))
```

This will print the following dictionary, e.g. for https://github.com/SeisSol/easi/blob/master/examples/1_groups.yaml and regions [2 1 1 2 1 1 1 2 1 1]:

```python
{'rho': array([2700., 2600., 2600., 2700., 2600., 2600., 2600., 2700., 2600.,
       2600.]), 'lambda': array([3.24038016e+10, 2.08000000e+10, 2.08000000e+10, 3.24038016e+10,
       2.08000000e+10, 2.08000000e+10, 2.08000000e+10, 3.24038016e+10,
       2.08000000e+10, 2.08000000e+10])}
```

Troubleshooting
---------------

An error `libnetcdf.so.18: undefined symbol: H5Pset_dxpl_mpio` at `import easi` is probably associated with a module loaded above depending on a conflicting netcdf or hdf5 library (e.g. vtk).

An error `ModuleNotFoundError: No module named 'easi'` may be cause by a $PYTHONPATH not pointing to the right folder, or a library compiled with the wrong python version (e.g. the library was compiled for the default python version and used with the Anaconda python).
