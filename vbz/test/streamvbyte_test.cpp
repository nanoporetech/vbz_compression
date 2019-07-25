#include "v0/vbz_streamvbyte.h"
#include "v1/vbz_streamvbyte.h"

#include "vbz.h"

#include "test_utils.h"

#include <numeric>
#include <random>

#include <catch2/catch.hpp>

struct StreamVByteFunctions
{
    using SizeFn = decltype(vbz_max_streamvbyte_compressed_size_v0)*;
    using CompressFn = decltype(vbz_delta_zig_zag_streamvbyte_compress_v0)*;
    using DecompressFn = decltype(vbz_delta_zig_zag_streamvbyte_decompress_v0)*;
    
    StreamVByteFunctions(
        SizeFn _size,
        CompressFn _compress,
        DecompressFn _decompress
    )
    : size(_size)
    , compress(_compress)
    , decompress(_decompress)
    {
    }

    SizeFn size;
    CompressFn compress;
    DecompressFn decompress;
};

StreamVByteFunctions const v0_functions{
    vbz_max_streamvbyte_compressed_size_v0,
    vbz_delta_zig_zag_streamvbyte_compress_v0,
    vbz_delta_zig_zag_streamvbyte_decompress_v0
};
StreamVByteFunctions const v1_functions{
    vbz_max_streamvbyte_compressed_size_v1,
    vbz_delta_zig_zag_streamvbyte_compress_v1,
    vbz_delta_zig_zag_streamvbyte_decompress_v1
};

template <typename T>
void perform_streamvbyte_compression_test(
    StreamVByteFunctions fns,
    std::vector<T> const& data,
    bool use_delta_zig_zag)
{
    INFO("Original     " << dump_explicit<std::int64_t>(data));
    
    auto const integer_size = sizeof(T);
    auto const input_byte_count = data.size() * integer_size;
    std::vector<std::int8_t> dest_buffer(fns.size(integer_size, vbz_size_t(input_byte_count)));
    auto final_byte_count = fns.compress(
        data.data(),
        vbz_size_t(data.size() * sizeof(data[0])),
        dest_buffer.data(),
        vbz_size_t(dest_buffer.size()),
        integer_size,
        use_delta_zig_zag);
    if (vbz_is_error(final_byte_count))
    {
        FAIL("Got error from vbz_delta_zig_zag_streamvbyte_compress");
        return;
    }
    dest_buffer.resize(final_byte_count);

    std::vector<int8_t> decompressed_bytes(data.size() * sizeof(data[0]));
    auto decompressed_byte_count = fns.decompress(
        dest_buffer.data(),
        vbz_size_t(dest_buffer.size()),
        decompressed_bytes.data(),
        vbz_size_t(decompressed_bytes.size()),
        integer_size,
        use_delta_zig_zag
    );

    INFO("decompressed_bytes " << dump_explicit<std::int64_t>(decompressed_bytes));
    if (vbz_is_error(decompressed_byte_count))
    {
        FAIL("Got error from vbz_delta_zig_zag_streamvbyte_decompress");
        return;
    }
    
    decompressed_bytes.resize(decompressed_byte_count);
    auto decompressed = gsl::make_span(decompressed_bytes).as_span<T>();
    INFO("decompressed   " << dump_explicit<std::int64_t>(decompressed));

    THEN("Data is filtered and recovered correctly")
    {
        CHECK(decompressed == gsl::make_span(data));
    }
}

template <typename T>
void run_streamvbyte_compression_test_suite(StreamVByteFunctions fns)
{
    GIVEN("Simple data to compress")
    {
        std::vector<T> simple_data(100);
        std::iota(simple_data.begin(), simple_data.end(), 0);

        WHEN("Compressing with no delta zig zag")
        {
            perform_streamvbyte_compression_test(fns, simple_data, false);
        }

        WHEN("Compressing with delta zig zag")
        {
            perform_streamvbyte_compression_test(fns, simple_data, true);
        }
    }

    GIVEN("Random data to compress")
    {
        std::vector<T> random_data(1000 * 1000);
        auto seed = std::random_device()();
        INFO("Seed " << seed);
        std::default_random_engine rand(seed);
        // std::uniform_int_distribution<std::int8_t> has issues on some platforms - always use 32 bit engine
        std::uniform_int_distribution<std::int64_t> dist(std::numeric_limits<T>::min()/2, std::numeric_limits<T>::max()/2);
        for (auto& e : random_data)
        {
            e = T(dist(rand));
        }

        WHEN("Compressing data")
        {
            perform_streamvbyte_compression_test(fns, random_data, std::is_signed<T>::value);
        }
    }
}

template <typename T> void perform_int_compressed_value_test(
    StreamVByteFunctions const& fns,
    std::vector<T> const& input_values,
    bool perform_zig_zag,
    std::vector<std::int8_t> const& expected_compressed)
{
    GIVEN("A known set of input signed values")
    {
        WHEN("Compressing/decompressing the values")
        {
            perform_streamvbyte_compression_test(fns, input_values, true);
            perform_streamvbyte_compression_test(fns, input_values, false);
        }
        
        AND_WHEN("Compressing the values with delta zig zag")
        {
            std::vector<int8_t> dest_buffer(100);
            auto final_byte_count = fns.compress(
                input_values.data(),
                vbz_size_t(input_values.size() * sizeof(input_values[0])),
                dest_buffer.data(),
                vbz_size_t(dest_buffer.size()),
                sizeof(input_values[0]),
                perform_zig_zag);
            dest_buffer.resize(final_byte_count);
            
            THEN("The values are as expected")
            {
                INFO("Compressed " << dump_explicit<std::int64_t>(dest_buffer));
                INFO("Expected   " << dump_explicit<std::int64_t>(expected_compressed));
                CHECK(expected_compressed == dest_buffer);
            }
        }
    }
}

SCENARIO("streamvbyte int8 encoding")
{
    run_streamvbyte_compression_test_suite<std::int8_t>(v0_functions);
}

SCENARIO("streamvbyte int16 encoding")
{
    run_streamvbyte_compression_test_suite<std::int16_t>(v0_functions);
}

SCENARIO("streamvbyte int32 encoding")
{
    run_streamvbyte_compression_test_suite<std::int32_t>(v0_functions);
}

SCENARIO("streamvbyte uint8 encoding")
{
    run_streamvbyte_compression_test_suite<std::uint8_t>(v0_functions);
}

SCENARIO("streamvbyte uint16 encoding")
{
    run_streamvbyte_compression_test_suite<std::uint16_t>(v0_functions);
}

SCENARIO("streamvbyte uint32 encoding")
{
    run_streamvbyte_compression_test_suite<std::uint32_t>(v0_functions);
}

SCENARIO("streamvbyte int16 encoding with known values.")
{
    GIVEN("signed types functions")
    {
        std::vector<std::int16_t> const input_values{ 0, -1, 4, -9, 16, -25, 36, -49, 64, -81, 100 };
        
        GIVEN("v0 functions")
        {
            std::vector<int8_t> compressed_values_v0{ 0, 0, 20, 0, 1, 10, 25, 50, 81, 122, -87, -30, 33, 1, 106, 1 };
            perform_int_compressed_value_test(v0_functions, input_values, true, compressed_values_v0);
        }
        
        GIVEN("v1 functions")
        {
            std::vector<int8_t> compressed_values_v1{ 0, 0, 20, 0, 1, 10, 25, 50, 81, 122, -87, -30, 33, 1, 106, 1, };
            perform_int_compressed_value_test(v1_functions, input_values, true, compressed_values_v1);
        }
    }
                               

    GIVEN("unsigned signed types functions")
    {
        std::vector<std::uint16_t> input_values{ 0, 1, 4, 9, 16, 25, 36, 49, 64, 81, 100 };

        GIVEN("v0 functions")
        {
            std::vector<int8_t> compressed_values_v0{ 0, 0, 0, 0, 1, 4, 9, 16, 25, 36, 49, 64, 81, 100 };
            perform_int_compressed_value_test(v0_functions, input_values, false, compressed_values_v0);
        }

        GIVEN("v1 functions")
        {
            std::vector<int8_t> compressed_values_v1{ 0, 0, 0, 0, 1, 4, 9, 16, 25, 36, 49, 64, 81, 100 };
            perform_int_compressed_value_test(v1_functions, input_values, false, compressed_values_v1);
        }
    }
}
