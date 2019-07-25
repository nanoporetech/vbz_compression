#pragma once

#include "vbz/vbz_export.h"

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define VBZ_DEFAULT_VERSION 0

typedef uint32_t vbz_size_t;

#define VBZ_ZSTD_ERROR ((vbz_size_t)-1)
#define VBZ_STREAMVBYTE_INPUT_SIZE_ERROR ((vbz_size_t)-2)
#define VBZ_STREAMVBYTE_INTEGER_SIZE_ERROR ((vbz_size_t)-3)
#define VBZ_STREAMVBYTE_DESTINATION_SIZE_ERROR ((vbz_size_t)-4)
#define VBZ_STREAMVBYTE_STREAM_ERROR ((vbz_size_t)-5)
#define VBZ_VERSION_ERROR ((vbz_size_t)-6)
#define VBZ_FIRST_ERROR VBZ_VERSION_ERROR

struct CompressionOptions
{
    // Flag to indicate the data should be converted to delta
    // then have zig zag encoding applied.
    // This causes similar signed numbers close to zero to end
    // up close to zero in unsigned space, and compresses better
    // when performing variable integer compression. 
    bool perform_delta_zig_zag;
    // Used to select the variable integer compression technique
    // Should be one of 1, 2 or 4.
    // Using a level of 1 will cause no variable integer encoding
    // to be performed.
    unsigned int integer_size;
    // zstd compression to apply.
    // Should be in the range "ZSTD_minCLevel" to "ZSTD_maxCLevel".
    // 1 gives the best performance and still provides a sensible compression
    // higher numbers use more CPU time for higher compression ratios.
    // Passing 0 will cause zstd to not be applied to data.
    unsigned int zstd_compression_level;

    // version of vbz to apply.
    // Should be initialised to 'VBZ_DEFAULT_VERSION' for the best, newest compression.
    // of set to older values to decompress older streams.
    unsigned int vbz_version;
};

/// \brief Find if a return value from a function is an error value.
VBZ_EXPORT bool vbz_is_error(vbz_size_t result_value);

/// \brief Find a string description for an error value
VBZ_EXPORT char const* vbz_error_string(vbz_size_t error_value);

/// \brief Find a theoretical max size for compressed output size.
///        should be used to find the size of the destination buffer to allocate.
/// \param source_size      The size of the source buffer for compression in bytes.
/// \param options          The options which will be used to compress data.
VBZ_EXPORT vbz_size_t vbz_max_compressed_size(
    vbz_size_t source_size,
    CompressionOptions const* options);

/// \brief  Compress data into a provided output buffer
/// \param source               Source data for compression.
/// \param source_size          Source data size (in bytes)
/// \param destination          Destination buffer for compressed output.
/// \param destination_capacity Size of the destination buffer to write to (see #max_compressed_size)
/// \param options              Options controlling compression to apply.
/// \return The size of the compressed object in bytes, or an error code if something went wrong.
VBZ_EXPORT vbz_size_t vbz_compress(
    void const* source,
    vbz_size_t source_size,
    void* destination,
    vbz_size_t destination_capacity,
    CompressionOptions const* options);

/// \brief  Decompress data into a provided output buffer
/// \param source               Source compressed data for decompression.
/// \param source_size          Compressed Source data size (in bytes)
/// \param destination          Destination buffer for decompressed output.
/// \param destination_size     Size of the destination buffer to write to in bytes.
///                             This must be a multiple of integer_size, and equal to the number of
///                             expected output bytes exactly. The caller is expected to store this information alongside
///                             the compressed data.
/// \param options              Options controlling decompression to
///                             apply (must be the same as the arguments passed to #vbz_compress).
/// \return The size of the decompressed object in bytes (will equal destination_size unless an error occurs).
VBZ_EXPORT vbz_size_t vbz_decompress(
    void const* source,
    vbz_size_t source_size,
    void* destination,
    vbz_size_t destination_size,
    CompressionOptions const* options);

/// \brief Compress data into a provided output buffer, with the original size information stored.
/// \note Must decompress data with #vbz_decompress_sized.
/// \param source               Source data for compression.
/// \param source_size          Source data size (in bytes)
/// \param destination          Destination buffer for compressed output.
/// \param destination_capacity Size of the destination buffer to write to (see #max_compressed_size)
/// \param options              Options controlling compression to apply.
/// \return The size of the compressed object in bytes, or an error code if something went wrong.
VBZ_EXPORT vbz_size_t vbz_compress_sized(
    void const* source,
    vbz_size_t source_size,
    void* destination,
    vbz_size_t destination_capacity,
    CompressionOptions const* options);

/// \brief Decompress data into a provided output buffer, using size information stored with the compressed data.
/// \note Must decompress data stored with #vbz_compress_sized.
/// \param source               Source compressed data for decompression.
/// \param source_size          Compressed Source data size (in bytes)
/// \param destination          Destination buffer for decompressed output.
/// \param destination_capacity Capacity of the destination buffer, should be at least #vbz_max_decompressed_size bytes.
/// \param options              Options controlling decompression to
///                             apply (must be the same as the arguments passed to #vbz_compress_sized).
/// \return The size of the decompressed object in bytes, or an error code if something went wrong.
VBZ_EXPORT vbz_size_t vbz_decompress_sized(
    void const* source,
    vbz_size_t source_size,
    void* destination,
    vbz_size_t destination_capacity,
    CompressionOptions const* options);

/// \brief Find the size for a decompressed block.
///        should be used to find the size of the destination buffer to allocate for decompression.
/// \param source               Source compressed data for decompression.
/// \param source_size      The size of the compressed source buffer in bytes.
/// \param options          The options which will be used to decompress data.
VBZ_EXPORT vbz_size_t vbz_decompressed_size(
    void const* source,
    vbz_size_t source_size,
    CompressionOptions const* options);

#if defined(__cplusplus)
}
#endif
