#include "vbz_streamvbyte.h"
#include "vbz.h"

#include "test_utils.h"

#include <numeric>
#include <random>

#include <catch2/catch.hpp>

template <typename T>
void perform_streamvbyte_compression_test(
    std::vector<T> const& data,
    bool use_delta_zig_zag)
{
    INFO("Original     " << dump_explicit<std::int64_t>(data));
    
    auto const integer_size = sizeof(T);
    auto const input_byte_count = data.size() * integer_size;
    std::vector<std::int8_t> dest_buffer(vbz_max_streamvbyte_compressed_size(integer_size, vbz_size_t(input_byte_count)));
    auto final_byte_count = vbz_delta_zig_zag_streamvbyte_compress(
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
    auto decompressed_byte_count = vbz_delta_zig_zag_streamvbyte_decompress(
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
void run_streamvbyte_compression_test_suite()
{
    GIVEN("Simple data to compress")
    {
        std::vector<T> simple_data(100);
        std::iota(simple_data.begin(), simple_data.end(), 0);

        WHEN("Compressing with no delta zig zag")
        {
            perform_streamvbyte_compression_test(simple_data, false);
        }

        WHEN("Compressing with delta zig zag")
        {
            perform_streamvbyte_compression_test(simple_data, true);
        }
    }

    GIVEN("Random data to compress")
    {
        std::vector<T> random_data(1000 * 1000);
        auto seed = std::random_device()();
        INFO("Seed " << seed);
        std::default_random_engine rand(seed);
        // std::uniform_int_distribution<std::int8_t> has issues on some platforms - always use 32 bit engine
        std::uniform_int_distribution<std::int64_t> dist(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
        for (auto& e : random_data)
        {
            e = T(dist(rand));
        }

        WHEN("Compressing with no delta zig zag")
        {
            perform_streamvbyte_compression_test(random_data, false);
        }
        
        WHEN("Compressing with delta zig zag")
        {
            perform_streamvbyte_compression_test(random_data, true);
        }
    }
}

SCENARIO("streamvbyte int8 encoding without integer comression")
{
    run_streamvbyte_compression_test_suite<std::int8_t>();
}

SCENARIO("streamvbyte int16 encoding without integer comression")
{
    run_streamvbyte_compression_test_suite<std::int16_t>();
}

SCENARIO("streamvbyte int32 encoding without integer comression")
{
    run_streamvbyte_compression_test_suite<std::int32_t>();
}

SCENARIO("streamvbyte uint8 encoding without integer comression")
{
    run_streamvbyte_compression_test_suite<std::uint8_t>();
}

SCENARIO("streamvbyte uint16 encoding without integer comression")
{
    run_streamvbyte_compression_test_suite<std::uint16_t>();
}

SCENARIO("streamvbyte uint32 encoding without integer comression")
{
    run_streamvbyte_compression_test_suite<std::uint32_t>();
}

SCENARIO("streamvbyte int16 encoding with known values.")
{
    GIVEN("A known set of input signed values")
    {
        std::vector<std::int16_t> input_values{ 0, -1, 4, -9, 16, -25, 36, -49, 64, -81, 100 };
        
        WHEN("Compressing/decompressing the values")
        {
            perform_streamvbyte_compression_test(input_values, true);
            perform_streamvbyte_compression_test(input_values, false);
        }
        
        AND_WHEN("Compressing the values with delta zig zag")
        {
            std::vector<int8_t> dest_buffer(100);
            auto final_byte_count = vbz_delta_zig_zag_streamvbyte_compress(
                input_values.data(),
                vbz_size_t(input_values.size() * sizeof(input_values[0])),
                dest_buffer.data(),
                vbz_size_t(dest_buffer.size()),
                sizeof(input_values[0]),
                true);
            dest_buffer.resize(final_byte_count);
            
            THEN("The values are as expected")
            {
                std::vector<int8_t> compressed_values{ 0, 0, 20, 0, 1, 10, 25, 50, 81, 122, -87, -30, 33, 1, 106, 1 };
                
                INFO("Compressed " << dump_explicit<std::int64_t>(dest_buffer));
                INFO("Expected   " << dump_explicit<std::int64_t>(compressed_values));
                CHECK(compressed_values == dest_buffer);
            }
        }
    }

    GIVEN("A known set of input unsigned values")
    {
        std::vector<std::uint16_t> input_values{ 0, 1, 4, 9, 16, 25, 36, 49, 64, 81, 100 };
        
        WHEN("Compressing/decompressing the values")
        {
            perform_streamvbyte_compression_test(input_values, true);
            perform_streamvbyte_compression_test(input_values, false);
        }
        
        AND_WHEN("Compressing the values without delta zig zag")
        {
            std::vector<int8_t> dest_buffer(100);
            auto final_byte_count = vbz_delta_zig_zag_streamvbyte_compress(
                input_values.data(),
                vbz_size_t(input_values.size() * sizeof(input_values[0])),
                dest_buffer.data(),
                vbz_size_t(dest_buffer.size()),
                sizeof(input_values[0]),
                false);
            dest_buffer.resize(final_byte_count);
            
            THEN("The values are as expected")
            {
                std::vector<int8_t> compressed_values{ 0, 0, 0, 0, 1, 4, 9, 16, 25, 36, 49, 64, 81, 100 };
                
                INFO("Compressed " << dump_explicit<std::int64_t>(dest_buffer));
                INFO("Expected   " << dump_explicit<std::int64_t>(compressed_values));
                CHECK(compressed_values == dest_buffer);
            }
        }
    }
    
}
