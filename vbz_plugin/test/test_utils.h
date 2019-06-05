#pragma once

#include "hdf_id_helper.h"

#include <vector>

using namespace ont::hdf5;

template <typename T>
std::vector<T> read_1d_dataset(
    hid_t parent,
    char const* name,
    hid_t expected_type
    )
{
    if (H5Lexists(parent, name, H5P_DEFAULT) < 0)
    {
        return {};
    }

    auto dataset = IdRef::claim(H5Dopen(parent, name, H5P_DEFAULT));
    auto type = IdRef::claim(H5Dget_type(dataset.get()));
    auto dataspace = IdRef::claim(H5Dget_space(dataset.get()));

    if (H5Tequal(type.get(), expected_type) < 0)
    {
        return {};
    }

    const int ndims = H5Sget_simple_extent_ndims(dataspace.get());
    if (ndims != 1)
    {
        throw std::runtime_error("dataset isn't 1d");
    }

    hsize_t dims[1];
    H5Sget_simple_extent_dims(dataspace.get(), dims, NULL);

    std::vector<T> values(dims[0]);
    auto buffer_space = IdRef::claim(
        H5Screate_simple(1, dims, dims));
    if (H5Dread(
        dataset.get(),
        expected_type,
        buffer_space.get(),
        H5S_ALL,
        H5P_DEFAULT,
        values.data()) < 0)
    {
        return {};
    }
        
    return values;
}

IdRef create_dataset(
    hid_t parent,
    char const* name,
    hid_t type,
    std::size_t size,
    hid_t dataset_creation_properties)
{
    auto data_space = IdRef::claim(H5Screate(H5S_SIMPLE));
    hsize_t size_arr[] = { size };
    hsize_t max_size_arr[] = { size };
    if (H5Sset_extent_simple(
        data_space.get(),
        1,
        size_arr,
        max_size_arr) < 0)
    {
        return IdRef();
    }

    auto dataset_id = IdRef::claim(H5Dcreate(
        parent,
        name,
        type,
        data_space.get(),
        H5P_DEFAULT,
        dataset_creation_properties,
        H5P_DEFAULT
        )
    );

    return dataset_id;
}

template <typename T>
bool write_full_dataset(
    hid_t dataset,
    hid_t type,
    T const& data
)
{
    if (H5Dwrite(
        dataset,
        type,
        H5S_ALL,
        H5S_ALL,
        H5P_DEFAULT,
        data.data() 
        ) < 0)
    {
        return false;
    }
    return true;
}
