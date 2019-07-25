#pragma once

#include "vbz/vbz_export.h"
#include "vbz.h"

#include <cstddef>

// Version 1 of streamvbyte
//
// This method introduces half byte + zero byte compression.

/// \brief find the maximum size a compressed data stream
/// using streamvbyte compression could be.
/// \param integer_size     The input integer size in bytes.
/// \param source_size      The size of the input buffer, in bytes.
VBZ_EXPORT vbz_size_t vbz_max_streamvbyte_compressed_size_v1(
    size_t integer_size,
    vbz_size_t source_size);

/// \brief Encode the source data using a combination of delta zig zag + streamvbyte encoding.
/// \param source                       Source data for compression.
/// \param source_size                  Source data size (in bytes)
/// \param destination                  Destination buffer for compressed output.
/// \param destination_capacity         Size of the destination buffer to write to (see #max_streamvbyte_compressed_size)
/// \param integer_size                 Number of bytes per integer
/// \param use_delta_zig_zag_encoding   Control if the data should be delta-zig-zag encoded before streamvbyte encoding.
/// \return The number of bytes used to compress data into [destination].
VBZ_EXPORT vbz_size_t vbz_delta_zig_zag_streamvbyte_compress_v1(
    void const* source,
    vbz_size_t source_size,
    void* destination,
    vbz_size_t destination_capacity,
    int integer_size,
    bool use_delta_zig_zag_encoding);

/// \brief Decode the source data using a combination of delta zig zag + streamvbyte encoding.
/// \param source                       Source compressed data for decompression.
/// \param source_size                  Source data size (in bytes)
/// \param destination                  Destination buffer for decompressed output.
/// \param destination_size             Size of the destination buffer to write to in bytes.
///                                     This must be a multiple of integer_size, and equal to the number of
///                                     expected output bytes exactly. The caller is expected to store this information alongside
///                                     the compressed data.
/// \param integer_size                 Number of bytes per integer (must equal size used to compress)
/// \param use_delta_zig_zag_encoding   Control if the data should be delta-zig-zag encoded before streamvbyte encoding.
///                                     (must equal value used to compress).
/// \return The number of bytes used to decompress data into [destination].
VBZ_EXPORT vbz_size_t vbz_delta_zig_zag_streamvbyte_decompress_v1(
    void const* source,
    vbz_size_t source_size,
    void* destination,
    vbz_size_t destination_size,
    int integer_size,
    bool use_delta_zig_zag_encoding);