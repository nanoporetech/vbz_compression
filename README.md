HDF5 Plugins
------------


Development without conan
-------------------------

To develop the plugin without conan you need the following installed:

- cmake 3.11 (https://cmake.org/)

and the following c++ dependencies

- zstd development libraries available to cmake
- hdf5 development libraries available to cmake (required for testing)

Then configure the project using:
```bash
> mkdir build
> cd build
> cmake -DENABLE_CONAN=OFF ..
> make -j
```

Development with conan
----------------------

To develop the plugin without conan you need the following installed:

- cmake 3.11 (https://cmake.org/)
- conan (https://conan.io/)
- ONT artifactory (https://artifactory.oxfordnanolabs.local/artifactory/webapp/#/artifacts/browse/tree/General/ONT-Conan)

Then configure and build the project using:
```bash
> mkdir build
> cd build
> cmake -DENABLE_CONAN=ON ..
> make -j
```

Installation
------------

Place the plugin shared library at:
  - `linux/osx`: `"/usr/local/hdf5/lib/plugin"`
  - `windows`: `"%ALLUSERSPROFILE%/hdf5/lib/plugin"`

You can then use h5repack or h5py from the command line:
```bash
# Invoke h5repack to pack input.fast5 into output.fast5
#
# The integer values specify how the data is packed:
#   - 32020: The id of the filter to apply (vbz in this case)
#   - 5: The number of following arguments
#   - 0: Padding value for configuring filter version
#   - 0: Padding value for configuring filter version
#   - 2: Packing integers of size 2 bytes
#   - 1: Use zig zag encoding
#   - 1: Use zstd compression level 1
> h5repack -f UD=32020,5,0,0,2,1,1 input.fast5 output.fast5

# To compress 4 byte unsigned integers (no zig zag) with level 3 zstd you could use:
> h5repack -f UD=32020,5,0,0,4,0,3 input.h5 output.h5
```