# A python wrapper for easi

Install with `-DPYTHON_BINDINGS=ON` cmake option.

Add the library to your path:

```bash
export PYTHONPATH=your_build_folder>python_bindings:$PYTHONPATH
```

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

# Troubleshooting

an error `libnetcdf.so.18: undefined symbol: H5Pset_dxpl_mpio` at `import easi` is probably associated with a module loaded above depending on a conflicting netcdf or hdf5 library (e.g. vtk).
