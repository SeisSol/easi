Compiling dependencies
========================

1. Setting up compilers

E.g. on SupermucNG:

.. code-block:: bash

    module load intel mpi-intel
    export FC=mpif90
    export CXX=mpiCC
    export CC=mpicc

Alternatively for gcc compilers on SupermucNG:

.. code-block:: bash

    export FC=mpifc
    export CXX=mpigxx
    export CC=mpigcc

2. Asagi
 
See https://seissol.readthedocs.io/en/latest/asagi.html.

3. yaml-cpp

On SupermucNG and with intel compilers:

.. code-block:: bash

    module load yaml-cpp/0.6.3-intel19

    
Alternatively to build from source:

.. code-block:: bash

    git clone git@github.com:jbeder/yaml-cpp
    # git clone https://github.com/jbeder/yaml-cpp.git
    cd yaml-cpp
    git checkout yaml-cpp-0.6.3
    mkdir build && cd build
    cmake ..  -DCMAKE_INSTALL_PREFIX=$HOME
    make -j 4
    make install
    cd ../..
    

4. ImpalaJIT 

.. code-block:: bash

    git clone git@github.com:uphoffc/ImpalaJIT
    # git clone https://github.com/uphoffc/ImpalaJIT.git
    cd ImpalaJIT 
    mkdir build && cd build
    cmake ..  -DCMAKE_INSTALL_PREFIX=$HOME
    make -j 4
    make install
    cd ../..

5. Lua

.. code-block:: bash

    # cmake module on SupermucNG (cmake/3.16.5) does not support lua 5.4
    wget https://www.lua.org/ftp/lua-5.3.6.tar.gz
    tar -xzvf lua-5.3.6.tar.gz
    cd lua-5.3.6/
    make linux CC=mpicc
    make local
    cp -r install/* ~
    cd ..

