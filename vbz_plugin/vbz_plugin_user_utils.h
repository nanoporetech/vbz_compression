#pragma once

#include <hdf5.h>

extern "C" const void* vbz_plugin_info(void);

/// \brief Call to enable the vbz filter on the specified creation properties.
/// \param integer_size             Size of integer type to be compressed. Leave at 0 to extract this information from the hdf type.
/// \param use_zig_zag              Control if zig zag encoding should be used on the type. If integer_size is not specified then the
///                                 hdf type's signedness is used to fill in this field.
/// \param zstd_compression_level   Control the level of compression used to filter the dataset.hd
inline void vbz_filter_enable(hid_t creation_properties, unsigned int integer_size, bool use_zig_zag, unsigned int zstd_compression_level)
{
    unsigned int values[4] = {
        0,
        integer_size,
        use_zig_zag,
        zstd_compression_level
    };
    
    H5Pset_filter(creation_properties, FILTER_VBZ_ID, 0, 4, values);
}

inline bool vbz_register()
{
    int retval = H5Zregister(vbz_plugin_info());
    if (retval < 0)
    {
        return 0;
    }

    return 1;
}
