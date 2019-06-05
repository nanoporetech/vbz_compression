#include "test_utils.h"
#include "hdf_id_helper.h"
#include "vbz_plugin.h"
#include "vbz_plugin_user_utils.h"

#include <hdf5.h>
#include <catch2/catch.hpp>

#include <array>
#include <random>

template <typename T> void run_linear_test(hid_t type, std::size_t count)
{
    GIVEN("An empty hdf file and a random data set")
    {
        auto file_id = H5Fcreate("./test_file.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
        auto file = IdRef::claim(file_id);

        // Generate an incrementing sequence of integers
        std::vector<T> data(count);
        std::iota(data.begin(), data.end(), 0);

        WHEN("Inserting filtered data into file")
        {
            auto creation_properties = IdRef::claim(H5Pcreate(H5P_DATASET_CREATE));
            std::array<hsize_t, 1> chunk_sizes{ { count / 8 } };
            H5Pset_chunk(creation_properties.get(), int(chunk_sizes.size()), chunk_sizes.data());
            
            vbz_filter_enable(creation_properties.get(), sizeof(T), true, 5);

            auto dataset = create_dataset(file_id, "foo", type, data.size(), creation_properties.get());

            write_full_dataset(dataset.get(), type, data);

            THEN("Data is read back correctly")
            {
                auto read_data = read_1d_dataset<T>(file_id, "foo", type);
                CHECK(read_data == data);
            }
        }
    }
}

template <typename T> void run_random_test(hid_t type, std::size_t count)
{
    GIVEN("An empty hdf file and a random data set")
    {
        auto file_id = H5Fcreate("./test_file.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
        auto file = IdRef::claim(file_id);

        std::vector<T> data(count);

        std::random_device rd;
        std::default_random_engine random_engine(rd());
        std::uniform_int_distribution<T> dist(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
        for (auto& elem : data)
        {
            elem = dist(random_engine);
        }

        WHEN("Inserting filtered data into file")
        {
            auto creation_properties = IdRef::claim(H5Pcreate(H5P_DATASET_CREATE));
            std::array<hsize_t, 1> chunk_sizes{ { count / 8 } };
            H5Pset_chunk(creation_properties.get(), int(chunk_sizes.size()), chunk_sizes.data());
            vbz_filter_enable(creation_properties.get(), sizeof(T), true, 1);

            auto dataset = create_dataset(file_id, "foo", type, data.size(), creation_properties.get());

            write_full_dataset(dataset.get(), type, data);

            THEN("Data is read back correctly")
            {
                auto read_data = read_1d_dataset<T>(file_id, "foo", type);
                CHECK(read_data == data);
            }
        }
    }
}

SCENARIO("Using zstd filter on a int8 dataset")
{
    run_linear_test<std::int8_t>(H5T_NATIVE_INT8, 100);
    run_random_test<std::int8_t>(H5T_NATIVE_INT8, 10 * 1000 * 1000);
}

SCENARIO("Using zstd filter on a int16 dataset")
{
    run_linear_test<std::int16_t>(H5T_NATIVE_INT16, 100);
    run_random_test<std::int16_t>(H5T_NATIVE_INT16, 10 * 1000 * 1000);
}

SCENARIO("Using zstd filter on a int32 dataset")
{
    run_linear_test<std::int32_t>(H5T_NATIVE_INT32, 100);
    run_random_test<std::int32_t>(H5T_NATIVE_INT32, 10 * 1000 * 1000);
}

SCENARIO("Using zstd filter on a uint8 dataset")
{
    run_linear_test<std::uint8_t>(H5T_NATIVE_UINT8, 100);
    run_random_test<std::uint8_t>(H5T_NATIVE_UINT8, 10 * 1000 * 1000);
}

SCENARIO("Using zstd filter on a uint16 dataset")
{
    run_linear_test<std::uint16_t>(H5T_NATIVE_UINT16, 100);
    run_random_test<std::uint16_t>(H5T_NATIVE_UINT16, 10 * 1000 * 1000);
}


SCENARIO("Using zstd filter on a uint32 dataset")
{
    run_linear_test<std::uint32_t>(H5T_NATIVE_UINT32, 100);
    run_random_test<std::uint32_t>(H5T_NATIVE_UINT32, 10 * 1000 * 1000);
}

