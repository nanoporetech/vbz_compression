#pragma once

#include "vbz.h"

#include "streamvbyte.h"
#include "streamvbyte_zigzag.h"

#include <gsl/gsl>

#include <cassert>

#ifdef _MSC_VER
# define VBZ_RESTRICT __restrict
#else
# define VBZ_RESTRICT __restrict__
#endif

static inline uint32_t _decode_data(const uint8_t **dataPtrPtr, uint8_t code, uint8_t *data_shift) {
    uint32_t val;
    
    auto read_data = [&](uint8_t bits)
    {
        uint32_t return_val = 0;
        auto& current_shift = *data_shift;
        for (std::size_t i = 0; i < bits / 4; ++i)
        {
            if (current_shift == 8)
            {
                current_shift = 0;
                *dataPtrPtr += 1;
            }
            
            const uint8_t *dataPtr = *dataPtrPtr;
            auto val_bits = 0xf & (*dataPtr >> current_shift);
            return_val |= val_bits << (i*4);
            current_shift += 4;
        }
        return return_val;
    };
    
    if (code == 0) { // 0 bytes
        val = 0;
    } else if (code == 1) { // 1/2 byte
        val = read_data(4);
    } else if (code == 2) { // 1 byte
        val = read_data(8);
    } else if (code == 3) { // 2 bytes
        val = read_data(16);
    } else {
        assert(0 && "Unknown code");
        val = 0;
    }

    return val;
}
static const uint8_t *svb_decode_scalar(uint32_t *outPtr, const uint8_t *keyPtr,
                                        const uint8_t *dataPtr,
                                        uint32_t count) {
    if (count == 0)
        return dataPtr; // no reads or writes if no data
    
    uint8_t data_shift = 0;
    uint8_t shift = 0;
    uint32_t key = *keyPtr++;
    for (uint32_t c = 0; c < count; c++) {
        if (shift == 8) {
            shift = 0;
            key = *keyPtr++;
        }
        uint32_t val = _decode_data(&dataPtr, (key >> shift) & 0x3, &data_shift);
        *outPtr++ = val;
        shift += 2;
    }
    
    if (data_shift != 0)
    {
        dataPtr += 1;
    }
    
    return dataPtr; // pointer to first unused byte after end
}

static uint8_t _encode_data(uint32_t val, uint8_t *VBZ_RESTRICT *dataPtrPtr, uint8_t* data_shift) {
    uint8_t code;

    auto write_data = [&](uint32_t value, uint8_t bits) {

        auto& current_shift = *data_shift;
        for (std::size_t i = 0; i < bits / 4; ++i)
        {
            if (current_shift == 8)
            {
                current_shift = 0;
                *dataPtrPtr += 1;
            }

            auto val_masked = value & 0xf;
            value >>= 4;
            
            uint8_t *dataPtr = *dataPtrPtr;
            if (current_shift == 0)
            {
                *dataPtr = 0;
            }
            *dataPtr |= val_masked << current_shift;
            current_shift += 4;
        }
    };

    if (val == 0) { // 0 bytes
        code = 0;
    } else if (val < (1 << 4)) { // 1/2 byte
        write_data(val, 4);
        code = 1;
    } else if (val < (1 << 8)) { // 1 byte
        write_data(val, 8);
        code = 2;
    } else { // 2 bytes
        write_data(val, 16);
        code = 3;
    }

    return code;
}

static uint8_t *svb_encode_scalar(const uint32_t *in,
                                  uint8_t *VBZ_RESTRICT keyPtr,
                                  uint8_t *VBZ_RESTRICT dataPtr,
                                  uint32_t count) {
    if (count == 0)
        return dataPtr; // exit immediately if no data

    uint8_t data_shift = 0;
    uint8_t shift = 0; // cycles 0, 2, 4, 6, 0, 2, 4, 6, ...
    uint8_t key = 0;
    for (uint32_t c = 0; c < count; c++) {
        if (shift == 8) {
            shift = 0;
            *keyPtr++ = key;
            key = 0;
        }
        uint32_t val = in[c];
        uint8_t code = _encode_data(val, &dataPtr, &data_shift);
        key |= code << shift;
        shift += 2;
    }
    
    if (data_shift != 0)
    {
        dataPtr += 1;
    }

    *keyPtr = key;  // write last key (no increment needed)
    return dataPtr; // pointer to first unused data byte
}

vbz_size_t streamvbyte_encode_half(uint32_t const* input, uint32_t count, uint8_t* output)
{
    uint8_t *keyPtr = output;
    uint32_t keyLen = (count + 3) / 4;  // 2-bits rounded to full byte
    uint8_t *dataPtr = keyPtr + keyLen; // variable byte data after all keys
    
    return vbz_size_t(svb_encode_scalar(input, keyPtr, dataPtr, count) - output);
}

vbz_size_t streamvbyte_decode_half(uint8_t const* input, uint32_t* output, uint32_t count)
{
    uint8_t *keyPtr = (uint8_t*)input;
    uint32_t keyLen = (count + 3) / 4;  // 2-bits rounded to full byte
    uint8_t *dataPtr = keyPtr + keyLen; // variable byte data after all keys
    
    return vbz_size_t(svb_decode_scalar(
        output,
        keyPtr,
        dataPtr,
        count
        ) - input);
}


/// \brief Generic implementation, safe for all integer types, and platforms.
template <typename T, bool UseZigZag>
struct StreamVByteWorkerV1
{
    static vbz_size_t compress(gsl::span<char const> input_bytes, gsl::span<char> output)
    {
        auto const input = input_bytes.as_span<T const>();
        
        if (!UseZigZag)
        {
            auto input_buffer = cast<std::uint32_t>(input);
            return vbz_size_t(streamvbyte_encode_half(
                input_buffer.data(),
                std::uint32_t(input_buffer.size()),
                output.as_span<std::uint8_t>().data()
            ));
        }
        
        std::vector<std::int32_t> input_buffer = cast<std::int32_t>(input);
        std::vector<std::uint32_t> intermediate_buffer(input.size());
        zigzag_delta_encode(input_buffer.data(), intermediate_buffer.data(), input_buffer.size(), 0);

        return vbz_size_t(streamvbyte_encode_half(
            intermediate_buffer.data(),
            std::uint32_t(intermediate_buffer.size()),
            output.as_span<std::uint8_t>().data()
        ));
    }
    
    static vbz_size_t decompress(gsl::span<char const> input, gsl::span<char> output_bytes)
    {
        auto const output = output_bytes.as_span<T>();
        
        std::vector<std::uint32_t> intermediate_buffer(output.size());
        auto read_bytes = streamvbyte_decode_half(
            input.as_span<std::uint8_t const>().data(),
            intermediate_buffer.data(),
            vbz_size_t(intermediate_buffer.size())
        );
        if (read_bytes != input.size())
        {
            return VBZ_STREAMVBYTE_STREAM_ERROR;
        }
        
        if (!UseZigZag)
        {
            cast(gsl::make_span(intermediate_buffer), gsl::make_span(output));
            return vbz_size_t(output.size() * sizeof(T));
        }
        
        std::vector<std::int32_t> output_buffer(output.size());
        zigzag_delta_decode(intermediate_buffer.data(), output_buffer.data(), output_buffer.size(), 0);
        
        cast(gsl::make_span(output_buffer), gsl::make_span(output));
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
