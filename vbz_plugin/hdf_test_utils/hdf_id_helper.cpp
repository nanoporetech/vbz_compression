#include "hdf_id_helper.h"

#define THROW(_ex) throw _ex

#define THROW_ON_ERROR(code) \
    if (code < 0) THROW(Exception())

namespace ont { namespace hdf5 {

static_assert(std::is_same<Id,hid_t>::value, "Mismatched ID types");

IdRef IdRef::claim(Id id)
{
    if (id < 0 || H5Iis_valid(id) <= 0) {
        THROW(Exception());
    }
    return IdRef(id);
}

IdRef IdRef::ref(Id id)
{
    assert(id >= 0);
    THROW_ON_ERROR(H5Iinc_ref(id));
    return IdRef(id);
}

IdRef IdRef::global_ref(Id id)
{
    assert(id >= 0);

    // Increment the index here - never decrementing it.
    THROW_ON_ERROR(H5Iinc_ref(id));
    return global(id);
}

IdRef::IdRef(IdRef const& other)
    : m_id(other.m_id)
    , m_is_global_constant(other.m_is_global_constant)
{
    if (!m_is_global_constant && m_id >= 0) {
        THROW_ON_ERROR(H5Iinc_ref(m_id));
    }
}

IdRef& IdRef::operator=(IdRef const& other)
{
    if (!other.m_is_global_constant && other.m_id >= 0) {
        THROW_ON_ERROR(H5Iinc_ref(other.m_id));
    }
    if (!m_is_global_constant && m_id >= 0) {
        auto result = H5Idec_ref(m_id);
        if (result < 0) {
            // this will be logged by the auto-logging code
            // (see install_error_function)
            assert(false);
        }
    }
    m_is_global_constant = other.m_is_global_constant;
    m_id = other.m_id;
    return *this;
}

IdRef::~IdRef()
{
    if (!m_is_global_constant && m_id >= 0) {
        auto result = H5Idec_ref(m_id);
        if (result < 0) {
            // this will be logged by the auto-logging code
            // (see install_error_function)
            assert(false);
        }
    }
}

int IdRef::ref_count() const
{
    if (m_id < 0) {
        return 0;
    }
    return H5Iget_ref(m_id);
}

}}