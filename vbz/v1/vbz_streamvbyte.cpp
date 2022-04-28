#include "vbz_streamvbyte.h"
#include "vbz_streamvbyte_impl.h"
#include "../v0/vbz_streamvbyte_impl.h" // for 4 byte case
#include "vbz.h"

#include <gsl/gsl-lite.hpp>

vbz_size_t vbz_max_streamvbyte_compressed_size_v1(
    std::size_t integer_size,
    vbz_size_t source_size)
{
    if (source_size % integer_size != 0)
    {
        return VBZ_STREAMVBYTE_INPUT_SIZE_ERROR;
    }

    auto int_count = source_size / integer_size;
    return vbz_size_t(streamvbyte_max_compressedbytes(std::uint32_t(int_count)));
}

vbz_size_t vbz_delta_zig_zag_streamvbyte_compress_v1(
    void const* source,
    vbz_size_t source_size,
    void* destination,
    vbz_size_t destination_capacity,
    int integer_size,
    bool use_delta_zig_zag_encoding)
{
    if (source_size % integer_size != 0)
    {
        return VBZ_STREAMVBYTE_INPUT_SIZE_ERROR;
    }
    
    auto const input_span = gsl::make_span(static_cast<char const*>(source), source_size);
    auto const output_span = gsl::make_span(static_cast<char*>(destination), destination_capacity);
    switch(integer_size) {
        case 1: {
            if (use_delta_zig_zag_encoding) {
                return StreamVByteWorkerV1<std::int8_t, true>::compress(input_span, output_span);
            }
            else {
                return StreamVByteWorkerV1<std::int8_t, false>::compress(input_span, output_span);
            }
        }
        case 2: {
            if (use_delta_zig_zag_encoding) {
                return StreamVByteWorkerV0<std::int16_t, true>::compress(input_span, output_span);
            }
            else {
                return StreamVByteWorkerV0<std::int16_t, false>::compress(input_span, output_span);
            }
        }
        case 4: {
            if (use_delta_zig_zag_encoding) {
                return StreamVByteWorkerV0<std::int32_t, true>::compress(input_span, output_span);
            }
            else {
                return StreamVByteWorkerV0<std::int32_t, false>::compress(input_span, output_span);
            }
        }
        default:
            return VBZ_STREAMVBYTE_INTEGER_SIZE_ERROR;
    }
}

vbz_size_t vbz_delta_zig_zag_streamvbyte_decompress_v1(
    void const* source,
    vbz_size_t source_size,
    void* destination,
    vbz_size_t destination_size,
    int integer_size,
    bool use_delta_zig_zag_encoding)
{
    if (destination_size % integer_size != 0)
    {
        return VBZ_STREAMVBYTE_DESTINATION_SIZE_ERROR;
    }
    
    auto const input_span = gsl::make_span(static_cast<char const*>(source), source_size);
    auto const output_span = gsl::make_span(static_cast<char*>(destination), destination_size);
    switch(integer_size) {
        case 1: {
            if (use_delta_zig_zag_encoding) {
                return StreamVByteWorkerV1<std::int8_t, true>::decompress(input_span, output_span);
            }
            else {
                return StreamVByteWorkerV1<std::int8_t, false>::decompress(input_span, output_span);
            }
        }
        // Integers larger than 1 byte have been shown to perform better (with zstd) when using version 0 compression
        // likely the increased noise in the key section reduces compression efficiency negating the benefits of
        // compressing 1 byte values into halfs.
        case 2: {
            if (use_delta_zig_zag_encoding) {
                return StreamVByteWorkerV0<std::int16_t, true>::decompress(input_span, output_span);
            }
            else {
                return StreamVByteWorkerV0<std::int16_t, false>::decompress(input_span, output_span);
            }
        }
        case 4: {
            if (use_delta_zig_zag_encoding) {
                return StreamVByteWorkerV0<std::int32_t, true>::decompress(input_span, output_span);
            }
            else {
                return StreamVByteWorkerV0<std::int32_t, false>::decompress(input_span, output_span);
            }
        }
        default:
            return VBZ_STREAMVBYTE_INTEGER_SIZE_ERROR;
    }
}
