#pragma once

#include <dlfcn.h>

#include <iostream>

typedef int herr_t;
typedef int H5Z_filter_t;
typedef int htri_t;

#ifdef HDF5_1_10_BUILD
typedef int64_t hid_t;
#else
typedef int32_t hid_t;
#endif

#define H5E_DEFAULT         (hid_t)0
#define H5E_CANTREGISTER    (*hdf5_dynamic::H5E_CANTREGISTER_g)
#define H5Z_FLAG_REVERSE    0x0100	/*reverse direction; read	*/
#define H5Z_CLASS_T_VERS    (1)

#define H5E_ERR_CLS         (*hdf5_dynamic::H5E_ERR_CLS_g)
#define H5E_CALLBACK        (*hdf5_dynamic::H5E_CALLBACK_g)
#define H5E_PLINE           (*hdf5_dynamic::H5E_PLINE_g)

#define H5T_NATIVE_UCHAR    (*hdf5_dynamic::H5T_NATIVE_UCHAR_g)
#define H5T_NATIVE_UINT8    (*hdf5_dynamic::H5T_NATIVE_UINT8_g)
#define H5T_NATIVE_USHORT   (*hdf5_dynamic::H5T_NATIVE_USHORT_g)
#define H5T_NATIVE_UINT16   (*hdf5_dynamic::H5T_NATIVE_UINT16_g)
#define H5T_NATIVE_UINT     (*hdf5_dynamic::H5T_NATIVE_UINT_g)
#define H5T_NATIVE_UINT32   (*hdf5_dynamic::H5T_NATIVE_UINT32_g)

namespace hdf5_dynamic
{

void* lookup_symbol(char const* name)
{
    auto lib_handle = RTLD_DEFAULT;
    if (auto lib_name = getenv("HDF5_LIB_PATH"))
    {
        std::cout << "Lookup symbols in specific lib " << lib_name << std::endl;
        lib_handle = dlopen(lib_name, RTLD_LAZY|RTLD_GLOBAL);
        if (!lib_handle)
        {
            std::cerr << dlerror() << std::endl;
            std::abort();
        }
    }

    auto sym = dlsym(lib_handle, name);
    std::cout << "Lookup symbol " << name << ": " << sym << std::endl;
    if (!sym)
    {
        std::cerr << dlerror() << std::endl;
        std::abort();
    }
}

template<typename Signature> class FunctionLookup;

template<typename Ret, typename... Args>
class FunctionLookup<Ret(Args...)>
{
public:
    using FunctionPtrType = Ret(*)(Args...);
    
    static FunctionPtrType lookup(char const* name)
    {
        return (FunctionPtrType)lookup_symbol(name);
    }
};

template <typename T>
class GlobalLookup
{
public:
    static T* lookup(char const* name)
    {
        return (T*)lookup_symbol(name);
    }
};

typedef htri_t (*H5Z_can_apply_func_t)(hid_t dcpl_id, hid_t type_id, hid_t space_id);
typedef herr_t (*H5Z_set_local_func_t)(hid_t dcpl_id, hid_t type_id, hid_t space_id);
typedef size_t (*H5Z_func_t)(unsigned int flags, size_t cd_nelmts,
    const unsigned int cd_values[], size_t nbytes,
    size_t *buf_size, void **buf);

typedef struct H5Z_class_t {
    int version;                    /* Version number of the H5Z_class_t struct     */
    H5Z_filter_t id;                /* Filter ID number                             */
    unsigned encoder_present;       /* Does this filter have an encoder?            */
    unsigned decoder_present;       /* Does this filter have a decoder?             */
    const char	*name;              /* Comment for debugging                        */
    H5Z_can_apply_func_t can_apply; /* The "can apply" callback for a filter        */
    H5Z_set_local_func_t set_local; /* The "set local" callback for a filter        */
    H5Z_func_t filter;              /* The actual filter function                   */
} H5Z_class_t;

typedef enum H5PL_type_t {
    H5PL_TYPE_ERROR         = -1,   /* Error                */
    H5PL_TYPE_FILTER        =  0,   /* Filter               */
    H5PL_TYPE_NONE          =  1    /* This must be last!   */
} H5PL_type_t;

namespace function_defs
{
herr_t H5check_version(unsigned majnum, unsigned minnum, unsigned relnum);
herr_t H5Pget_filter_by_id2(hid_t plist_id, H5Z_filter_t id,
       unsigned int *flags/*out*/, size_t *cd_nelmts/*out*/,
       unsigned cd_values[]/*out*/, size_t namelen, char name[]/*out*/,
       unsigned *filter_config/*out*/);
herr_t H5Pmodify_filter(hid_t plist_id, H5Z_filter_t filter,
       unsigned int flags, size_t cd_nelmts,
       const unsigned int cd_values[/*cd_nelmts*/]);
herr_t H5Zregister(const void *cls);
size_t H5Tget_size(hid_t type_id);
size_t H5Tget_size(hid_t type_id);
herr_t H5Epush2(hid_t err_stack, const char *file, const char *func, unsigned line,
    hid_t cls_id, hid_t maj_id, hid_t min_id, const char *msg, ...);
}

static auto H5check_version = FunctionLookup<decltype(function_defs::H5check_version)>::lookup("H5check_version");
static auto H5Pget_filter_by_id2 = FunctionLookup<decltype(function_defs::H5Pget_filter_by_id2)>::lookup("H5Pget_filter_by_id2");
static auto H5Pmodify_filter = FunctionLookup<decltype(function_defs::H5Pmodify_filter)>::lookup("H5Pmodify_filter");
static auto H5Zregister = FunctionLookup<decltype(function_defs::H5Zregister)>::lookup("H5Zregister");
static auto H5Tget_size = FunctionLookup<decltype(function_defs::H5Tget_size)>::lookup("H5Tget_size");

// Uses c varargs so a bit trickier to templatise
using H5Epush2Type = decltype(function_defs::H5Epush2);
static auto H5Epush2 = (H5Epush2Type*)lookup_symbol("H5Epush2");

hid_t* H5E_ERR_CLS_g = GlobalLookup<hid_t>::lookup("H5E_ERR_CLS_g");
hid_t* H5E_CANTREGISTER_g = GlobalLookup<hid_t>::lookup("H5E_CANTREGISTER_g");
hid_t* H5E_CALLBACK_g = GlobalLookup<hid_t>::lookup("H5E_CALLBACK_g");
hid_t* H5E_PLINE_g = GlobalLookup<hid_t>::lookup("H5E_PLINE_g");

hid_t* H5T_NATIVE_UCHAR_g = GlobalLookup<hid_t>::lookup("H5T_NATIVE_UCHAR_g");
hid_t* H5T_NATIVE_UINT8_g = GlobalLookup<hid_t>::lookup("H5T_NATIVE_UINT8_g");
hid_t* H5T_NATIVE_USHORT_g = GlobalLookup<hid_t>::lookup("H5T_NATIVE_USHORT_g");
hid_t* H5T_NATIVE_UINT16_g = GlobalLookup<hid_t>::lookup("H5T_NATIVE_UINT16_g");
hid_t* H5T_NATIVE_UINT_g = GlobalLookup<hid_t>::lookup("H5T_NATIVE_UINT_g");
hid_t* H5T_NATIVE_UINT32_g = GlobalLookup<hid_t>::lookup("H5T_NATIVE_UINT32_g");

}