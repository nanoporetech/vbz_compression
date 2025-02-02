#pragma once

#include "vbz.h"

#include "streamvbyte.h"
#include "streamvbyte_zigzag.h"

#include <gsl/gsl-lite.hpp>

#include <algorithm>
#include <cstdint>
#include <vector>

/// \brief Generic implementation, safe for all integer types, and platforms.
template <typename T, bool UseZigZag>
struct StreamVByteWorkerV0
{
    static vbz_size_t compress(gsl::span<char const> input_bytes, gsl::span<char> output)
    {
        auto const input = input_bytes.as_span<T const>();
        
        if (!UseZigZag)
        {
            auto input_buffer = cast<std::uint32_t>(input);
            return vbz_size_t(streamvbyte_encode(
                input_buffer.data(),
                std::uint32_t(input_buffer.size()),
                output.as_span<std::uint8_t>().data()
            ));
        }
        
        std::vector<std::int32_t> input_buffer = cast<std::int32_t>(input);
        std::vector<std::uint32_t> intermediate_buffer(input.size());
        zigzag_delta_encode(input_buffer.data(), intermediate_buffer.data(), input_buffer.size(), 0);

        return vbz_size_t(streamvbyte_encode(
            intermediate_buffer.data(),
            std::uint32_t(intermediate_buffer.size()),
            output.as_span<std::uint8_t>().data()
        ));
    }
    
    static vbz_size_t decompress(gsl::span<char const> input, gsl::span<char> output_bytes)
    {
        auto const output = output_bytes.as_span<T>();
        auto in_data = input.as_span<std::uint8_t const>().data();
        auto const out_size = vbz_size_t(output.size());

        if (!streamvbyte_validate_stream(in_data, input.size_bytes(), out_size)) {
            return VBZ_STREAMVBYTE_STREAM_ERROR;
        }

        // streamvbyte requires additional padding, so copy to a temporary buffer that has that
        std::vector<uint8_t> in_temp(input.size_bytes() + STREAMVBYTE_PADDING);
        std::copy_n(in_data, input.size_bytes(), in_temp.begin());
        in_data = in_temp.data();

        std::vector<std::uint32_t> intermediate_buffer(out_size);
        auto read_bytes = streamvbyte_decode(
            in_data,
            intermediate_buffer.data(),
            out_size
        );
        if (read_bytes != input.size())
        {
            return VBZ_STREAMVBYTE_STREAM_ERROR;
        }
        
        if (!UseZigZag)
        {
            cast(gsl::make_span(intermediate_buffer), output);
            return vbz_size_t(output.size() * sizeof(T));
        }
        
        std::vector<std::int32_t> output_buffer(output.size());
        zigzag_delta_decode(intermediate_buffer.data(), output_buffer.data(), output_buffer.size(), 0);
        
        cast(gsl::make_span(output_buffer), output);
        return vbz_size_t(output.size() * sizeof(T));
    }
    
    template <typename U, typename V>
    static std::vector<U> cast(gsl::span<V> const& input)
    {
        std::vector<U> output(input.size());
        for (std::size_t i = 0; i < input.size(); ++i)
        {
            output[i] = input[i];
        }
        return output;
    }
    
    template <typename U, typename V>
    static void cast(gsl::span<U> input, gsl::span<V> output)
    {
        for (std::size_t i = 0; i < input.size(); ++i)
        {
            output[i] = input[i];
        }
    }
};

#ifdef __SSE3__

#include "vbz_streamvbyte_impl_sse3.h"

#endif
