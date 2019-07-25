"""
Python bindings for libvbz
"""

__version__ = "0.9.3"


import numpy as np

from _vbz import lib, ffi


def compression_options(zigzag, size, zlevel=1, version=0):
    options = ffi.new("CompressionOptions *")
    options.integer_size = size
    options.perform_delta_zig_zag = zigzag
    options.zstd_compression_level = zlevel
    options.vbz_version = version
    return options


def compress(data, options=None):

    if options is None:
        zigzag = np.issubdtype(data.dtype, np.signedinteger)
        options = compression_options(zigzag, data.dtype.itemsize)

    output_size = lib.vbz_max_compressed_size(len(data) * options.integer_size, options)
    output = np.empty(output_size, dtype=np.uint8)

    if lib.vbz_is_error(output_size):
        raise Exception("Something unexpected went wrong")

    size = lib.vbz_compress_sized(
        ffi.cast("void const *", ffi.from_buffer(data)),
        len(data) * options.integer_size,
        ffi.cast("void *", ffi.from_buffer(output)),
        output_size,
        options
    )

    if lib.vbz_is_error(size):
        raise Exception("Something unexpected went wrong")

    return output[:size]


def decompress(data, dtype, options=None):

    if options is None:
        zigzag = np.issubdtype(dtype, np.signedinteger)
        try:
            options = compression_options(zigzag, dtype.itemsize)
        except TypeError:
            options = compression_options(zigzag, dtype().itemsize)

    uncompressed_size = lib.vbz_decompressed_size(
        ffi.cast("void const *", ffi.from_buffer(data)),
        len(data),
        options
    )

    if lib.vbz_is_error(uncompressed_size):
        raise Exception("Something unexpected went wrong")

    output = np.empty(uncompressed_size, dtype=dtype)

    size = lib.vbz_decompress_sized(
        ffi.cast("void const *", ffi.from_buffer(data)),
        len(data),
        ffi.cast("void *", ffi.from_buffer(output)),
        uncompressed_size,
        options
    )

    if lib.vbz_is_error(size):
        raise Exception("Something unexpected went wrong")

    return output[:int(size / options.integer_size)]
