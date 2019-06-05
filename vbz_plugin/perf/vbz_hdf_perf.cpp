#include "../test/test_utils.h"
#include "../../vbz/perf/test_data_generator.h"

#include "hdf_id_helper.h"
#include "vbz_plugin.h"
#include "vbz_plugin_user_utils.h"

#include <hdf5.h>
#include <catch2/catch.hpp>

#include <array>

#include <benchmark/benchmark.h>

template <typename IntType>
hid_t get_h5_type()
{
    hid_t type = 0;
    if (std::is_same<IntType, std::int8_t>::value)
    {
        type = H5T_NATIVE_UINT8;
    }
    else if (std::is_same<IntType, std::uint8_t>::value)
    {
        type = H5T_NATIVE_INT8;
    }
    else if (std::is_same<IntType, std::int16_t>::value)
    {
        type = H5T_NATIVE_UINT16;
    }
    else if (std::is_same<IntType, std::uint16_t>::value)
    {
        type = H5T_NATIVE_INT16;
    }
    else if (std::is_same<IntType, std::int32_t>::value)
    {
        type = H5T_NATIVE_UINT32;
    }
    else if (std::is_same<IntType, std::uint32_t>::value)
    {
        type = H5T_NATIVE_INT32;
    }
    else
    {
        std::abort();
    }
    return type;
}

template <bool UseZigZag, std::size_t ZstdLevel>
void vbz_filter(hid_t creation_properties, int int_size)
{
    vbz_filter_enable(creation_properties, int_size, UseZigZag, ZstdLevel);
}

void zlib_filter(hid_t creation_properties, int)
{
    H5Pset_deflate(creation_properties, 1);
}

void no_filter(hid_t creation_properties, int)
{
}

using FilterSetupFn = decltype(no_filter)*;

template <typename Generator>
void vbz_hdf_benchmark(benchmark::State& state, int integer_size, hid_t h5_type, FilterSetupFn setup_filter)
{
    std::size_t max_element_count = 0;
    auto input_value_list = Generator::generate(max_element_count);
    
    std::size_t item_count = 0;
    for (auto _ : state)
    {
        std::size_t id = 0;
        
        state.PauseTiming();
        auto file_id = H5Fcreate("./test_file.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
        auto file = IdRef::claim(file_id);
        state.ResumeTiming();
        
        item_count = 0;
        for (auto const& input_values : input_value_list)
        {
            auto creation_properties = IdRef::claim(H5Pcreate(H5P_DATASET_CREATE));
            
            std::array<hsize_t, 1> chunk_sizes{ { input_values.size() } };
            H5Pset_chunk(creation_properties.get(), int(chunk_sizes.size()), chunk_sizes.data());
            
            setup_filter(creation_properties.get(), integer_size);

            std::string dset_name = std::to_string(id++);
            auto dataset = create_dataset(file_id, dset_name.c_str(), h5_type, input_values.size(), creation_properties.get());
            
            auto val = write_full_dataset(dataset.get(), h5_type, input_values);
            item_count += input_values.size();
            benchmark::DoNotOptimize(val);
        }
    }

    state.SetItemsProcessed(state.iterations() * item_count);
    state.SetBytesProcessed(state.iterations() * item_count * integer_size);
}

template <typename IntType, int ZstdLevel>
void vbz_hdf_benchmark_sequence(benchmark::State& state)
{
    vbz_hdf_benchmark<SequenceGenerator<IntType>>(state, sizeof(IntType), get_h5_type<IntType>(), vbz_filter<true, ZstdLevel>);
}

template <typename IntType>
void vbz_hdf_benchmark_sequence_uncompressed(benchmark::State& state)
{
    vbz_hdf_benchmark<SequenceGenerator<IntType>>(state, sizeof(IntType), get_h5_type<IntType>(), no_filter);
}

template <typename IntType>
void vbz_hdf_benchmark_sequence_zlib(benchmark::State& state)
{
    vbz_hdf_benchmark<SequenceGenerator<IntType>>(state, sizeof(IntType), get_h5_type<IntType>(), zlib_filter);
}

template <typename IntType, int ZstdLevel>
void vbz_hdf_benchmark_random(benchmark::State& state)
{
    vbz_hdf_benchmark<SignalGenerator<IntType>>(state, sizeof(IntType), get_h5_type<IntType>(), vbz_filter<true, ZstdLevel>);
}

template <typename IntType>
void vbz_hdf_benchmark_random_uncompressed(benchmark::State& state)
{
    vbz_hdf_benchmark<SignalGenerator<IntType>>(state, sizeof(IntType), get_h5_type<IntType>(), no_filter);
}

template <typename IntType>
void vbz_hdf_benchmark_random_zlib(benchmark::State& state)
{
    vbz_hdf_benchmark<SignalGenerator<IntType>>(state, sizeof(IntType), get_h5_type<IntType>(), zlib_filter);
}

/*BENCHMARK_TEMPLATE2(vbz_hdf_benchmark_sequence, std::int8_t, 0);
BENCHMARK_TEMPLATE2(vbz_hdf_benchmark_sequence, std::int16_t, 0);
BENCHMARK_TEMPLATE2(vbz_hdf_benchmark_sequence, std::int32_t, 0);
BENCHMARK_TEMPLATE2(vbz_hdf_benchmark_sequence, std::int8_t, 1);
BENCHMARK_TEMPLATE2(vbz_hdf_benchmark_sequence, std::int16_t, 1);
BENCHMARK_TEMPLATE2(vbz_hdf_benchmark_sequence, std::int32_t, 1);

BENCHMARK_TEMPLATE(vbz_hdf_benchmark_sequence_uncompressed, std::int8_t);
BENCHMARK_TEMPLATE(vbz_hdf_benchmark_sequence_uncompressed, std::int16_t);
BENCHMARK_TEMPLATE(vbz_hdf_benchmark_sequence_uncompressed, std::int32_t);
BENCHMARK_TEMPLATE(vbz_hdf_benchmark_sequence_zlib, std::int8_t);
BENCHMARK_TEMPLATE(vbz_hdf_benchmark_sequence_zlib, std::int16_t);
BENCHMARK_TEMPLATE(vbz_hdf_benchmark_sequence_zlib, std::int32_t);*/

BENCHMARK_TEMPLATE2(vbz_hdf_benchmark_random, std::int8_t, 0);
BENCHMARK_TEMPLATE2(vbz_hdf_benchmark_random, std::int16_t, 0);
BENCHMARK_TEMPLATE2(vbz_hdf_benchmark_random, std::int32_t, 0);
BENCHMARK_TEMPLATE2(vbz_hdf_benchmark_random, std::int8_t, 1);
BENCHMARK_TEMPLATE2(vbz_hdf_benchmark_random, std::int16_t, 1);
BENCHMARK_TEMPLATE2(vbz_hdf_benchmark_random, std::int32_t, 1);

/*
BENCHMARK_TEMPLATE(vbz_hdf_benchmark_random_uncompressed, std::int8_t);
BENCHMARK_TEMPLATE(vbz_hdf_benchmark_random_uncompressed, std::int16_t);
BENCHMARK_TEMPLATE(vbz_hdf_benchmark_random_uncompressed, std::int32_t);
BENCHMARK_TEMPLATE(vbz_hdf_benchmark_random_zlib, std::int8_t);
BENCHMARK_TEMPLATE(vbz_hdf_benchmark_random_zlib, std::int16_t);
BENCHMARK_TEMPLATE(vbz_hdf_benchmark_random_zlib, std::int32_t);
*/

static bool plugin_init_result = vbz_register();

// Run the benchmark
BENCHMARK_MAIN();
