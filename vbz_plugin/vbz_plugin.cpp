#include "vbz_plugin/vbz_hdf_plugin_export.h"
#include "vbz_plugin.h"
#include "vbz.h"

#include <gsl/gsl-lite.hpp>
#include <hdf5/hdf5_plugin_types.h>

#include <array>
#include <chrono>
#include <iostream>
#include <memory>

#ifdef _WIN32
# ifndef NOMINMAX
#  define NOMINMAX
#endif
# include <Windows.h>
#endif

#define VBZ_DEBUG 0

namespace {
#if VBZ_DEBUG
int checksum(gsl::span<char const> input)
{
    std::uint8_t output = 0x0;
    for (auto x : input)
    {
        output ^= x;
    }
    return output;
}
#endif

#if defined(_WIN32) && !defined(HDF5_USE_STATIC_LIBRARIES)
HMODULE get_hdf_module()
{
    static HMODULE module = nullptr;
    if (!module)
    {
        auto module_name = "hdf5.dll";
        auto env_val = getenv("VBZ_DEBUG_HDF");
        if (env_val && strcmp(env_val, "1") == 0)
        {
            module_name = "hdf5_D.dll";
        }

        module = LoadLibraryA(module_name);
    }

    if (!module)
    {
        std::cerr << "Failed to load hdf library" << std::endl;
        std::abort();
    }
    return module;
}

#endif

// Windows ends up using different runtimes/heaps depending on which
// _python_ version hdf5 plugin was built against, the simple way to fix this
// is to use whatever hdf5 uses for allocating memory.
//
void* h5_malloc(std::size_t size)
{
#if defined(_WIN32) && !defined(HDF5_USE_STATIC_LIBRARIES)
    static auto module = get_hdf_module();

    static auto malloc_memory = (void*(*)(size_t, bool))GetProcAddress(module, "H5allocate_memory");
    return malloc_memory(size, false);
#else
    return malloc(size);
#endif
}

void h5_free(void* memory)
{
#if defined(_WIN32) && !defined(HDF5_USE_STATIC_LIBRARIES)
    static auto module = get_hdf_module();

    static auto free_hdf5 = (int(*)(void *))GetProcAddress(module, "H5free_memory");
    free_hdf5(memory);
#else
    free(memory);
#endif
}

struct h5free_delete
{
    void operator()(void* x) { h5_free(x); }
};


}

size_t vbz_filter(
    unsigned flags,
    size_t cd_nelmts,
    const unsigned int cd_values[],
    size_t nbytes,
    size_t* buf_size,
    void** buf)
{
    std::unique_ptr<void, h5free_delete> outbuf;
    vbz_size_t outbuf_size = 0;
    vbz_size_t outbuf_used_size = 0;

    if (cd_nelmts < 3)
    {
        return 0;
    }

    unsigned int vbz_version = cd_values[FILTER_VBZ_VERSION_OPTION];
    unsigned int integer_size = cd_values[FILTER_VBZ_INTEGER_SIZE_OPTION];
    bool use_zig_zag = cd_values[FILTER_VBZ_USE_DELTA_ZIG_ZAG_COMPRESSION] != 0;

    unsigned int compression_level = 1;
    if (cd_nelmts > FILTER_VBZ_ZSTD_COMPRESSION_LEVEL_OPTION)
    {
        compression_level = cd_values[FILTER_VBZ_ZSTD_COMPRESSION_LEVEL_OPTION];
    }
    
    CompressionOptions options{ use_zig_zag, integer_size, compression_level, vbz_version };
    
#if VBZ_DEBUG
    std::cout << "======================================================\n"
        << "Using options:"
        << " integer_size: " << integer_size
        << " use_zig_zag: " << use_zig_zag
        << " compression_level: " << compression_level
        << std::endl;
#endif

    // If decompressing
    if (flags & H5Z_FLAG_REVERSE)
    {
        auto input_span = gsl::make_span(static_cast<char*>(*buf), *buf_size);
        if (input_span.size() > std::numeric_limits<vbz_size_t>::max())
        {
            std::cerr << "vbz_filter: Chunk size too large." << std::endl;
            return 0;
        }

#if VBZ_DEBUG
        std::cout << "Decmpressing data with checksum " << checksum(input_span) << std::endl;
#endif

        auto const expected_uncompressed_size = vbz_decompressed_size(
            input_span.data(),
            vbz_size_t(input_span.size()),
            &options);
        if (vbz_is_error(expected_uncompressed_size))
        {
            std::cerr << "vbz_filter: size error" << std::endl;
            return 0;
        }
        outbuf.reset(h5_malloc(expected_uncompressed_size));

        outbuf_used_size = vbz_decompress_sized(
            input_span.data(),
            vbz_size_t(input_span.size()),
            outbuf.get(),
            expected_uncompressed_size,
            &options);
        if (vbz_is_error(outbuf_used_size))
        {
            std::cerr << "vbz_filter: compression error" << std::endl;
            return 0;
        }
        
        if (outbuf_used_size != expected_uncompressed_size)
        {
            std::cerr << "vbz_filter: decompressed size error" << std::endl;
            return 0;
        }

#if VBZ_DEBUG
        std::cout << "Decompressed dataset from " << *buf_size << "  bytes to " << outbuf_used_size
            << " with checksum " << checksum(gsl::make_span(static_cast<char*>(outbuf.get()), outbuf_used_size)) << std::endl;
#endif
    }
    else // compressing
    {
#if VBZ_DEBUG
        std::cout << "Compressing data with checksum " << checksum(gsl::make_span(static_cast<char*>(*buf), *buf_size)) << std::endl;
#endif
        if (*buf_size > std::numeric_limits<vbz_size_t>::max())
        {
            std::cerr << "vbz_filter: Chunk size too large." << std::endl;
            return 0;
        }

        auto const byte_remainder = *buf_size % integer_size;
        if (byte_remainder != 0)
        {
            std::cerr << "vbz_filter: Invalid integer_size specified" << std::endl;
            return 0;
        }

        outbuf_size = vbz_max_compressed_size(vbz_size_t(*buf_size), &options);
        outbuf.reset(h5_malloc(outbuf_size));
        
        auto output_span = gsl::make_span(static_cast<char*>(outbuf.get()), outbuf_size);

        // do compress
        outbuf_used_size += vbz_compress_sized(
            *buf,
            vbz_size_t(*buf_size),
            output_span.data(),
            vbz_size_t(output_span.size()),
            &options
        );
        if (vbz_is_error(outbuf_used_size))
        {
            std::cerr << "vbz_filter: compression error" << std::endl;;
            return 0;
        }

#if VBZ_DEBUG
        std::cout << "Compressed dataset from " << *buf_size << "  bytes to " << outbuf_used_size << " with checksum " << checksum(gsl::make_span(output_span.data(), outbuf_used_size)) << std::endl;
#endif
    }    

    h5_free(*buf);
    *buf = outbuf.release();
    *buf_size = outbuf_size;
    return outbuf_used_size;
}

H5Z_class2_t const vbz_filter_struct = {
    H5Z_CLASS_T_VERS,   // version
    FILTER_VBZ_ID,      // id
    1,                  // encoder_present
    1,                  // decoder_present
    "vbz",              // name
    nullptr,            // can_apply
    nullptr,            // set_local
    vbz_filter          // filter
};

extern "C" VBZ_HDF_PLUGIN_EXPORT const void* vbz_plugin_info(void)
{
    return &vbz_filter_struct;
}

// hdf plugin hooks
extern "C" VBZ_HDF_PLUGIN_EXPORT H5PL_type_t H5PLget_plugin_type(void)
{
    return H5PL_TYPE_FILTER;
}

// hdf plugin hooks
extern "C" VBZ_HDF_PLUGIN_EXPORT const void* H5PLget_plugin_info(void)
{
#if VBZ_DEBUG
    std::cout << "Registering vbz plugin" << std::endl;
#endif

    return vbz_plugin_info();
}
