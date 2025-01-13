#include <limits>
#include <vbz.h>

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <numeric>
#include <utility>
#include <vector>

// Enable debug log messages.
#define DEBUG_LOGGING 0

// Speed up the decompress tests, at the cost of potentially missing out of bounds reads/writes.
// Only seems to impact sanitizers on macOS.
#ifdef __APPLE__
#define REUSE_DECOMPRESS_BUFFER 1
#else
#define REUSE_DECOMPRESS_BUFFER 0
#endif

namespace {

template <bool ShouldLog = DEBUG_LOGGING, typename... Args>
void debug_log(Args&&... args)
{
    if (ShouldLog) {
        (std::cout << ... << std::forward<Args>(args)) << std::endl;
    }
}

#define REQUIRE(x, ...) \
    if (!(x)) { \
        debug_log<true>("Check on line ", __LINE__, " failed: " #x, __VA_ARGS__); \
        std::abort(); \
    }

// For fatal errors
#define REQUIRE_NO_VBZ_ERROR(x) \
    do { \
        vbz_size_t const _x = x; \
        REQUIRE(!vbz_is_error(_x), #x "=", _x, ", vbz_error=", vbz_error_string(_x)); \
    } while (false)

// For non-fatal errors (will return on error)
#define RETURN_IF_VBZ_ERROR(x) \
    do { \
        vbz_size_t const _x = x; \
        if (vbz_is_error(_x)) { \
            return; \
        } \
    } while (false)

void compare(const uint8_t* expected_data, std::size_t expected_size, const uint8_t* actual_data, std::size_t actual_size) {
    REQUIRE(expected_size == actual_size, "expected_size=", expected_size, ", actual_size=", actual_size);
    for (std::size_t i{}; i < actual_size; ++i) {
        REQUIRE(expected_data[i] == actual_data[i], "i=", i, ", expected_data=", expected_data, ", actual_data=", actual_data);
    }
}

template <bool Sized>
void run_vbz_compress_test(const uint8_t* data, vbz_size_t size, vbz_size_t max_size, CompressionOptions const& options) {
    auto compressor = Sized ? vbz_compress_sized : vbz_compress;
    auto decompressor = Sized ? vbz_decompress_sized : vbz_decompress;

    // Compress the data.
    auto compressed = std::vector<uint8_t>(max_size);
    auto const compressed_size = compressor(data, size, compressed.data(), max_size, &options);
    // This can fail under certain circumstances (incorrect padding for options, not enough memory, etc...) so it's not fatal.
    RETURN_IF_VBZ_ERROR(compressed_size);
    REQUIRE(compressed_size <= compressed.size(), "compressed_size=", compressed_size, ", compressed.size()=", compressed.size());

    debug_log("compress_sized: Compressed to ", compressed_size, " bytes");

    // Not all platforms have container bounds checking so allocate a new vector of the exact size.
    compressed = std::vector<uint8_t>{compressed.begin(), compressed.begin() + compressed_size};

    // Decompress the data.
    auto decompressed = std::vector<uint8_t>(size);
    auto const decompressed_size = decompressor(compressed.data(), compressed_size, decompressed.data(), size, &options);
    // The decompression shouldn't fail.
    REQUIRE_NO_VBZ_ERROR(decompressed_size);
    REQUIRE(decompressed_size == size, "decompressed_size=", decompressed_size, ", size=", size);

    debug_log("compress: Decompressed to ", decompressed_size, " bytes");

    // Check that what we got out matches what we put in.
    compare(data, size, decompressed.data(), decompressed_size);
}

void run_vbz_compress_tests(const uint8_t* data, vbz_size_t size, CompressionOptions const& options) {
    auto const max_size = vbz_max_compressed_size(size, &options);
    RETURN_IF_VBZ_ERROR(max_size);

    // Run sized and unsized versions.
    run_vbz_compress_test<false>(data, size, max_size, options);
    run_vbz_compress_test<true>(data, size, max_size, options);
}

void run_vbz_decompress_test(const uint8_t* data, vbz_size_t size, vbz_size_t original_size, std::vector<uint8_t> & decompress_dest, CompressionOptions const& options)
{
#if REUSE_DECOMPRESS_BUFFER
    decompress_dest.resize(original_size);
#else
    decompress_dest = std::vector<uint8_t>(original_size);
#endif

    // Unsized version.
    {
        auto const decompressed_size = vbz_decompress(data, size, decompress_dest.data(), original_size, &options);
        if (vbz_is_error(decompressed_size))
        {
            debug_log("decompress: Error in decompressed_size ", vbz_error_string(decompressed_size));
        }
    }

    // Sized version.
    {
        auto const decompressed_size = vbz_decompress_sized(data, size, decompress_dest.data(), original_size, &options);
        if (vbz_is_error(decompressed_size))
        {
            debug_log("decompress_sized: Error in decompressed_size ", vbz_error_string(decompressed_size));
        }
    }

    // Extract size.
    {
        auto const decompressed_size = vbz_decompressed_size(data, size, &options);
        if (vbz_is_error(decompressed_size))
        {
            debug_log("decompressed_size: Error in decompressed_size ", vbz_error_string(decompressed_size));
        }
    }
}

void run_vbz_decompress_tests(const uint8_t* data, vbz_size_t size, CompressionOptions const& options) {
    constexpr std::size_t max_guess_size = 1024 * 1024;
    std::vector<uint8_t> decompress_dest;
#if REUSE_DECOMPRESS_BUFFER
    decompress_dest.reserve(max_guess_size);
#endif

    // vbz doesn't offer a way to get the exact size of compressed data, so find the maximum input size that
    // would compress to given output.
    std::size_t max_destination_size{0};
    for (std::size_t candidate_decompress_size{1}; candidate_decompress_size <= max_guess_size; candidate_decompress_size *= 2) {
        auto max_compressed_size = vbz_max_compressed_size((vbz_size_t)candidate_decompress_size, &options);
        if (max_compressed_size > size) {
            max_destination_size = candidate_decompress_size;
            break;
        }
    }

    // Try to decompress the data with all of those sizes.
    for (std::size_t guess_destination_size = 0; guess_destination_size <= max_destination_size; guess_destination_size++) {
        debug_log("guess_destination_size ", guess_destination_size);
        run_vbz_decompress_test(data, size, (vbz_size_t)guess_destination_size, decompress_dest, options);
    }
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Skip sizes that are too big.
    if (size > std::numeric_limits<vbz_size_t>::max()) {
        return 0;
    }

    debug_log("Begin with ", size, " bytes");

    for (bool perform_delta_zig_zag : {true, false}) {
        for (unsigned integer_size : {0, 1, 2, 4}) {
            for (unsigned zstd_compression_level : {0, 1}) {
                for (unsigned vbz_version : {0, 1}) {
                    debug_log("Running with perform_delta_zig_zag=", perform_delta_zig_zag,
                        ", integer_size=", integer_size,
                        ", zstd_compression_level=", zstd_compression_level,
                        ", vbz_version=", vbz_version
                    );
                    CompressionOptions const options{
                        perform_delta_zig_zag,
                        integer_size,
                        zstd_compression_level,
                        vbz_version,
                    };
                    run_vbz_compress_tests(data, (vbz_size_t)size, options);
                    run_vbz_decompress_tests(data, (vbz_size_t)size, options);
                }
            }
        }
    }

    return 0;
}
