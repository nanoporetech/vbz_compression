"""
Builds the C Python wrapper for libvbz
"""

import os
from cffi import FFI


vbz_include_paths = os.environ["VBZ_INCLUDE_PATHS"].split(";")
vbz_libs = os.environ["VBZ_LINK_LIBS"].split(";")


ffibuilder = FFI()


ffibuilder.set_source(
    "_vbz",
    """
    #include <vbz.h>
    """,
    include_dirs=vbz_include_paths,
    extra_objects=vbz_libs,
    source_extension='.cpp',
    libraries=['c', 'stdc++'],
    extra_compile_args=['-std=c++11'],
)


ffibuilder.cdef("""
typedef uint32_t vbz_size_t;

typedef struct {
    bool perform_delta_zig_zag;
    unsigned int integer_size;
    unsigned int zstd_compression_level;
    unsigned int vbz_version;
} CompressionOptions;

bool vbz_is_error(vbz_size_t result_value);

vbz_size_t vbz_max_compressed_size(
    vbz_size_t source_size,
    CompressionOptions const* options
);

vbz_size_t vbz_decompressed_size(
    void const* source,
    vbz_size_t source_size,
    CompressionOptions const* options
);

vbz_size_t vbz_compress_sized(
    void const* source,
    vbz_size_t source_size,
    void* destination,
    vbz_size_t destination_capacity,
    CompressionOptions const* options
);

vbz_size_t vbz_decompress_sized(
    void const* source,
    vbz_size_t source_size,
    void* destination,
    vbz_size_t destination_capacity,
    CompressionOptions const* options
);
""")


if __name__ == "__main__":
    ffibuilder.compile()
