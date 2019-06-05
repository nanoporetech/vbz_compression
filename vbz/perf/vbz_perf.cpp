#include "vbz.h"
#include "test_data_generator.h"

#include <benchmark/benchmark.h>

template <typename VbzOptions, typename Generator>
void streamvbyte_compress_benchmark(benchmark::State& state)
{
    std::size_t max_element_count = 0;
    auto input_value_list = Generator::generate(max_element_count);

    auto const int_size = sizeof(typename VbzOptions::IntType);
    
    CompressionOptions options{
        VbzOptions::UseZigZag,
        int_size,
        VbzOptions::ZstdLevel
    };
    
    std::vector<char> dest_buffer(vbz_max_compressed_size(vbz_size_t(max_element_count * int_size), &options));

    std::size_t item_count = 0;
    for (auto _ : state)
    {
        item_count = 0;
        for (auto const& input_values : input_value_list)
        {
            auto const input_byte_count = input_values.size() * sizeof(input_values[0]);
            item_count += input_values.size();
            dest_buffer.resize(dest_buffer.capacity());

            
            auto bytes_used = vbz_compress(
                input_values.data(),
                vbz_size_t(input_byte_count),
                dest_buffer.data(),
                vbz_size_t(dest_buffer.size()),
                &options);

            benchmark::DoNotOptimize(bytes_used);
        }
    }

    state.SetItemsProcessed(state.iterations() * item_count);
    state.SetBytesProcessed(state.iterations() * item_count * int_size);
}

template <typename VbzOptions, typename Generator>
void streamvbyte_decompress_benchmark(benchmark::State& state)
{
    std::size_t max_element_count = 0;
    auto input_value_list = Generator::generate(max_element_count);
    
    auto const int_size = sizeof(typename VbzOptions::IntType);
    
    CompressionOptions options{
        VbzOptions::UseZigZag,
        int_size,
        VbzOptions::ZstdLevel
    };
    
    std::vector<char> compressed_buffer(vbz_max_compressed_size(vbz_size_t(max_element_count * int_size), &options));
    std::vector<char> dest_buffer(max_element_count * int_size);

    std::size_t item_count = 0;
    for (auto _ : state)
    {
        item_count = 0;
        for (auto const& input_values : input_value_list)
        {
            state.PauseTiming();
            auto const input_byte_count = input_values.size() * sizeof(input_values[0]);
            item_count += input_values.size();
            compressed_buffer.resize(compressed_buffer.capacity());
            dest_buffer.resize(input_byte_count);

            auto compressed_used_bytes = vbz_compress(
                input_values.data(),
                vbz_size_t(input_byte_count),
                compressed_buffer.data(),
                vbz_size_t(compressed_buffer.size()),
                &options
            );

            state.ResumeTiming();

            auto bytes_expanded_to = vbz_decompress(
                compressed_buffer.data(),
                compressed_used_bytes,
                dest_buffer.data(),
                vbz_size_t(dest_buffer.size()),
                &options
            );
            assert(bytes_expanded_to == input_byte_count);

            benchmark::DoNotOptimize(bytes_expanded_to);
        }
    }

    state.SetItemsProcessed(state.iterations() * item_count);
    state.SetBytesProcessed(state.iterations() * item_count * int_size);
}

template <typename _IntType>
struct VbzNoZStd
{
    using IntType = _IntType;
    static const std::size_t UseZigZag = 1;
    static const std::size_t ZstdLevel = 0;
};

template <typename _IntType>
struct VbzZStd
{
    using IntType = _IntType;
    static const std::size_t UseZigZag = 1;
    static const std::size_t ZstdLevel = 1;
};

template <typename CompressionOptions>
void compress_sequence(benchmark::State& state)
{
    streamvbyte_compress_benchmark<CompressionOptions, SequenceGenerator<typename CompressionOptions::IntType>>(state);
}

template <typename CompressionOptions>
void compress_random(benchmark::State& state)
{
    streamvbyte_compress_benchmark<CompressionOptions, SignalGenerator<typename CompressionOptions::IntType>>(state);
}

template <typename CompressionOptions>
void decompress_sequence(benchmark::State& state)
{
    streamvbyte_decompress_benchmark<CompressionOptions, SequenceGenerator<typename CompressionOptions::IntType>>(state);
}

template <typename CompressionOptions>
void decompress_random(benchmark::State& state)
{
    streamvbyte_decompress_benchmark<CompressionOptions, SignalGenerator<typename CompressionOptions::IntType>>(state);
}

BENCHMARK_TEMPLATE(compress_sequence, VbzZStd<std::int8_t>);
BENCHMARK_TEMPLATE(compress_sequence, VbzZStd<std::int16_t>);
BENCHMARK_TEMPLATE(compress_sequence, VbzZStd<std::int32_t>);
BENCHMARK_TEMPLATE(compress_sequence, VbzNoZStd<std::int8_t>);
BENCHMARK_TEMPLATE(compress_sequence, VbzNoZStd<std::int16_t>);
BENCHMARK_TEMPLATE(compress_sequence, VbzNoZStd<std::int32_t>);

BENCHMARK_TEMPLATE(compress_random, VbzZStd<std::int8_t>);
BENCHMARK_TEMPLATE(compress_random, VbzZStd<std::int16_t>);
BENCHMARK_TEMPLATE(compress_random, VbzZStd<std::int32_t>);
BENCHMARK_TEMPLATE(compress_random, VbzNoZStd<std::int8_t>);
BENCHMARK_TEMPLATE(compress_random, VbzNoZStd<std::int16_t>);
BENCHMARK_TEMPLATE(compress_random, VbzNoZStd<std::int32_t>);

BENCHMARK_TEMPLATE(decompress_sequence, VbzZStd<std::int8_t>);
BENCHMARK_TEMPLATE(decompress_sequence, VbzZStd<std::int16_t>);
BENCHMARK_TEMPLATE(decompress_sequence, VbzZStd<std::int32_t>);
BENCHMARK_TEMPLATE(decompress_sequence, VbzNoZStd<std::int8_t>);
BENCHMARK_TEMPLATE(decompress_sequence, VbzNoZStd<std::int16_t>);
BENCHMARK_TEMPLATE(decompress_sequence, VbzNoZStd<std::int32_t>);

BENCHMARK_TEMPLATE(decompress_random, VbzZStd<std::int8_t>);
BENCHMARK_TEMPLATE(decompress_random, VbzZStd<std::int16_t>);
BENCHMARK_TEMPLATE(decompress_random, VbzZStd<std::int32_t>);
BENCHMARK_TEMPLATE(decompress_random, VbzNoZStd<std::int8_t>);
BENCHMARK_TEMPLATE(decompress_random, VbzNoZStd<std::int16_t>);
BENCHMARK_TEMPLATE(decompress_random, VbzNoZStd<std::int32_t>);


// Run the benchmark
BENCHMARK_MAIN();
