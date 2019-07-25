# pyvbz

Python bindings for `libvbz`.

## Installation

Python wheels are provided with each [release](https://github.com/nanoporetech/vbz_compression/releases) and can be install with with `pip`.

```
$ pip install https://github.com/nanoporetech/vbz_compression/releases/download/v1.0.0/pyvbz-1.0.0-cp35-cp35m-linux_x86_64.whl
```

## Example

```python
>>> import vbz
>>> import numpy as np
>>> signal = np.arange(0, 1000, dtype=np.int16)                                                         
>>> signal.nbytes                                                                                       
2000
>>> compressed = vbz.compress(signal)                                                                   
>>> compressed.nbytes                                                                                   
27
>>> recovered = vbz.decompress(compressed, dtype=np.int16)
>>> all(signal == recovered)
True
```
