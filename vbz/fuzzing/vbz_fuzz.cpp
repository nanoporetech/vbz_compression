#include <vbz.h>

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <utility>
#include <vector>

#define DEBUG_LOGGING 0

template <typename... Args>
void debug_log(Args&&... args)
{
#if DEBUG_LOGGING
    using expander = int[];
    (void)expander{0, (void(std::cout << std::forward<Args>(args)), 0)...};
    std::cout << std::endl;
#endif
}

void run_vbz_test(const uint8_t* data, size_t size, CompressionOptions const& options)
{
    // Try to compress random input data
    {
        auto max_size = vbz_max_compressed_size((vbz_size_t)size, &options);
        if (vbz_is_error(max_size))
        {
            debug_log("compress: Error in size ", vbz_error_string(max_size));
            max_size = size * 100; // make up a max value...
        }

        auto compressed = std::vector<char>(max_size);
        // run both sized and non-sized methods.
        auto compressed_size = vbz_compress_sized(data, size, compressed.data(), max_size, &options);
        if (vbz_is_error(compressed_size))
        {
            debug_log("compress_sized: Error in compressed_size", vbz_error_string(compressed_size));
        }
        else
        {
            debug_log("compress_sized: Compressed to ", compressed_size, " bytes");
            compressed.resize(compressed_size);
        }

        compressed_size = vbz_compress(data, size, compressed.data(), max_size, &options);
        if (vbz_is_error(compressed_size))
        {
            debug_log("compress: Error in compressed_size", vbz_error_string(compressed_size));
        }
        else
        {
            debug_log("compress: Compressed to ", compressed_size, " bytes");
            compressed.resize(compressed_size);
        }

        auto decompressed = std::vector<char>(size);
        auto decompressed_size = vbz_decompress(compressed.data(), compressed.size(), decompressed.data(), size, &options);
        if (vbz_is_error(decompressed_size))
        {
            debug_log("compress_sized: Error in decompressed_size ",  vbz_error_string(decompressed_size));
        }
        else
        {
            debug_log("compress_sized: Decompressed to ", decompressed_size, " bytes");
        }

        decompressed_size = vbz_decompress_sized(compressed.data(), compressed.size(), decompressed.data(), size, &options);
        if (vbz_is_error(decompressed_size))
        {
            debug_log("compress: Error in decompressed_size ",  vbz_error_string(decompressed_size));
        }
        else
        {
            debug_log("compress: Decompressed to ", decompressed_size, " bytes");
        }
    }

    // Try to decompress random input data...
    {
        auto const guess_destination_size = std::max(std::size_t(1024), size * 100);
        auto decompress_dest = std::vector<char>(guess_destination_size);
        // try both sized and non-sized methods.
        auto decompressed_size = vbz_decompress(data, size, decompress_dest.data(), guess_destination_size, &options);
        if (vbz_is_error(decompressed_size))
        {
            debug_log("decompress_sized: Error in decompressed_size ", vbz_error_string(decompressed_size));
        }
        decompressed_size = vbz_decompress_sized(data, size, decompress_dest.data(), guess_destination_size, &options);
        if (vbz_is_error(decompressed_size))
        {
            debug_log("decompress: Error in decompressed_size ", vbz_error_string(decompressed_size));
        }
    }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    debug_log("Begin with ", size, " bytes");

    // Run with zstd
    run_vbz_test(data, size, CompressionOptions{ true, 2, 1, VBZ_DEFAULT_VERSION });

    // Run without zstd
    run_vbz_test(data, size, CompressionOptions{ true, 2, 0, VBZ_DEFAULT_VERSION });
    
    return 0;
}