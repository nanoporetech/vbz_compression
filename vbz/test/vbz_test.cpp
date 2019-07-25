#include <cstddef>
#include <iostream>
#include <numeric>
#include <random>

#include "vbz.h"
#include "test_utils.h"

#include "test_data.h"

#include <catch2/catch.hpp>

template <typename T>
void perform_compression_test(
    std::vector<T> const& data,
    CompressionOptions const& options)
{
    auto const input_data_size = vbz_size_t(data.size() * sizeof(data[0]));
    std::vector<int8_t> dest_buffer(vbz_max_compressed_size(input_data_size, &options));
    auto final_byte_count = vbz_compress(
        data.data(),
        input_data_size,
        dest_buffer.data(),
        vbz_size_t(dest_buffer.size()),
        &options);
    REQUIRE(!vbz_is_error(final_byte_count));
    dest_buffer.resize(final_byte_count);

    std::vector<int8_t> decompressed_bytes(input_data_size);
    auto decompressed_byte_count = vbz_decompress(
        dest_buffer.data(),
        vbz_size_t(dest_buffer.size()),
        decompressed_bytes.data(),
        vbz_size_t(decompressed_bytes.size()),
        &options
    );
    REQUIRE(!vbz_is_error(decompressed_byte_count));
    decompressed_bytes.resize(decompressed_byte_count);
    auto decompressed = gsl::make_span(decompressed_bytes).as_span<T>();

    //INFO("Original     " << dump_explicit<std::int64_t>(data));
    //INFO("Decompressed " << dump_explicit<std::int64_t>(decompressed));
    CHECK(decompressed == gsl::make_span(data));
}

template <typename T>
void run_compression_test_suite()
{
    GIVEN("Simple data to compress with no delta-zig-zag")
    {
        std::vector<T> simple_data(100);
        std::iota(simple_data.begin(), simple_data.end(), 0);
        
        CompressionOptions simple_options{
            false, // no delta zig zag
            sizeof(T),
            1,
            VBZ_DEFAULT_VERSION
        };
        
        perform_compression_test(simple_data, simple_options);
    }

    GIVEN("Simple data to compress and applying delta zig zag")
    {
        std::vector<T> simple_data(100);
        std::iota(simple_data.begin(), simple_data.end(), 0);

        CompressionOptions simple_options{
            true,
            sizeof(T),
            1,
            VBZ_DEFAULT_VERSION
        };

        perform_compression_test(simple_data, simple_options);
    }
    
    GIVEN("Simple data to compress with delta-zig-zag and no zstd")
    {
        std::vector<T> simple_data(100);
        std::iota(simple_data.begin(), simple_data.end(), 0);
        
        CompressionOptions simple_options{
            true,
            sizeof(T),
            0,
            VBZ_DEFAULT_VERSION
        };
        
        perform_compression_test(simple_data, simple_options);
    }

    GIVEN("Random data to compress")
    {
        std::vector<T> random_data(10 * 1000);
        auto seed = std::random_device()();
        INFO("Seed " << seed);
        std::default_random_engine rand(seed);
        // std::uniform_int_distribution<std::int8_t> has issues on some platforms - always use 32 bit engine
        std::uniform_int_distribution<std::int32_t> dist(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
        for (auto& e : random_data)
        {
            e = dist(rand);
        }
        
        WHEN("Compressing with no delta zig zag")
        {
            CompressionOptions options{
                false,
                sizeof(T),
                1,
                VBZ_DEFAULT_VERSION
            };
            perform_compression_test(random_data, options);
        }
        
        WHEN("Compressing with delta zig zag")
        {
            CompressionOptions options{
                true,
                sizeof(T),
                0,
                VBZ_DEFAULT_VERSION
            };
            
            perform_compression_test(random_data, options);
        }
        
        WHEN("Compressing with zstd and delta zig zag")
        {
            CompressionOptions options{
                true,
                sizeof(T),
                1,
                VBZ_DEFAULT_VERSION
            };
            
            perform_compression_test(random_data, options);
        }
    }
}

struct InputStruct
{
    std::uint32_t size = 100;
    unsigned char keys[25] = {
        0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff,
    };
};

SCENARIO("vbz int8 encoding")
{
    run_compression_test_suite<std::int8_t>();
}

SCENARIO("vbz int16 encoding")
{
    run_compression_test_suite<std::int16_t>();
}

SCENARIO("vbz int32 encoding")
{
    run_compression_test_suite<std::int32_t>();
}


SCENARIO("vbz int32 known input data")
{
    GIVEN("A known input data set")
    {
        std::vector<std::int32_t> simple_data{ 5, 4, 3, 2, 1 };
        
        WHEN("Compressed without zstd, with delta zig-zag")
        {
            CompressionOptions simple_options{
                true,
                sizeof(simple_data[0]),
                0,
                VBZ_DEFAULT_VERSION
            };
            
            THEN("Data compresses/decompresses as expected")
            {
                perform_compression_test(simple_data, simple_options);
            }
            
            AND_WHEN("Checking compressed data")
            {
                auto const input_data_size = vbz_size_t(simple_data.size() * sizeof(simple_data[0]));
                std::vector<int8_t> dest_buffer(vbz_max_compressed_size(input_data_size, &simple_options));
                auto final_byte_count = vbz_compress(
                    simple_data.data(),
                    input_data_size,
                    dest_buffer.data(),
                    vbz_size_t(dest_buffer.size()),
                    &simple_options);
                dest_buffer.resize(final_byte_count);
                
                std::vector<int8_t> expected{ 0, 0, 10, 1, 1, 1, 1, };
                
                INFO("Compressed   " << dump_explicit<std::int64_t>(dest_buffer));
                INFO("Decompressed " << dump_explicit<std::int64_t>(expected));
                CHECK(dest_buffer == expected);
            }
        }
        
        WHEN("Compressed with zstd and delta zig-zag")
        {
            CompressionOptions simple_options{
                true,
                sizeof(simple_data[0]),
                100,
                VBZ_DEFAULT_VERSION
            };
            
            THEN("Data compresses/decompresses as expected")
            {
                perform_compression_test(simple_data, simple_options);
            }
            
            AND_WHEN("Checking compressed data")
            {
                auto const input_data_size = vbz_size_t(simple_data.size() * sizeof(simple_data[0]));
                std::vector<int8_t> dest_buffer(vbz_max_compressed_size(input_data_size, &simple_options));
                auto final_byte_count = vbz_compress(
                    simple_data.data(),
                    input_data_size,
                    dest_buffer.data(),
                    vbz_size_t(dest_buffer.size()),
                    &simple_options);
                dest_buffer.resize(final_byte_count);
                
                std::vector<int8_t> expected{ 40, -75, 47, -3, 32, 7, 57, 0, 0, 0, 0, 10, 1, 1, 1, 1, };
                
                INFO("Compressed   " << dump_explicit<std::int64_t>(dest_buffer));
                INFO("Decompressed " << dump_explicit<std::int64_t>(expected));
                CHECK(dest_buffer == expected);
            }
        }
    }
}

SCENARIO("vbz int16 known input large data")
{
    GIVEN("Test data from a realistic dataset")
    {
        WHEN("Compressing with zig-zag deltas")
        {
            CompressionOptions options{
                true,
                sizeof(test_data[0]),
                0,
                VBZ_DEFAULT_VERSION
            };
            
            perform_compression_test(test_data, options);
        }

        WHEN("Compressing with zstd")
        {
            CompressionOptions options{
                true,
                sizeof(test_data[0]),
                1,
                VBZ_DEFAULT_VERSION
            };
            
            perform_compression_test(test_data, options);
        }

        WHEN("Compressing with no options")
        {
            CompressionOptions options{
                false,
                1,
                0,
                VBZ_DEFAULT_VERSION
            };
            
            perform_compression_test(test_data, options);
        }
    }
}

SCENARIO("vbz sized compression")
{
    GIVEN("A known input data set")
    {
        std::vector<std::int32_t> simple_data{ 5, 4, 3, 2, 1 };
        
        WHEN("Compressed without zstd, with delta zig-zag")
        {
            CompressionOptions simple_options{
                true,
                sizeof(simple_data[0]),
                0,
                VBZ_DEFAULT_VERSION
            };
            
            
            WHEN("Compressing data")
            {
                auto const input_data_size = vbz_size_t(simple_data.size() * sizeof(simple_data[0]));
                std::vector<int8_t> compressed_buffer(vbz_max_compressed_size(input_data_size, &simple_options));
                
                auto final_byte_count = vbz_compress_sized(
                    simple_data.data(),
                    input_data_size,
                    compressed_buffer.data(),
                    vbz_size_t(compressed_buffer.size()),
                    &simple_options);
                compressed_buffer.resize(final_byte_count);
                
                THEN("Data is compressed correctly")
                {
                    std::vector<int8_t> expected{ 20, 0, 0, 0, 0, 0, 10, 1, 1, 1, 1, };
                    
                    INFO("Compressed   " << dump_explicit<std::int64_t>(compressed_buffer));
                    INFO("Decompressed " << dump_explicit<std::int64_t>(expected));
                    CHECK(compressed_buffer == expected);
                }
                
                AND_WHEN("Decompressing data")
                {
                    std::vector<std::int8_t> dest_buffer(
                        vbz_decompressed_size(compressed_buffer.data(),
                            vbz_size_t(compressed_buffer.size()),
                            &simple_options)
                    );
                    CHECK(dest_buffer.size() == input_data_size);
                    
                    auto final_byte_count = vbz_decompress_sized(
                        compressed_buffer.data(),
                        vbz_size_t(compressed_buffer.size()),
                        dest_buffer.data(),
                        vbz_size_t(dest_buffer.size()),
                        &simple_options);
                    CHECK(final_byte_count == input_data_size);
                    
                    CHECK(gsl::make_span(dest_buffer).as_span<std::int32_t>() == gsl::make_span(simple_data));
                }
            }
        }
    }
}
