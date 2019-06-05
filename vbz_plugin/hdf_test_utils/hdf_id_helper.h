#pragma once

#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <utility>

#include <hdf5.h>

namespace ont { namespace hdf5 {

/// Thrown when something goes wrong internally in HDF5.
class Exception : public std::runtime_error
{
public:
    Exception()
        : std::runtime_error("HDF5 exception")
    {
    }
};

/// An HDF5 identifier.
///
/// IdRef should be used by anything that wants to keep a reference to a HDF5
/// object.
using Id = hid_t;

/// Maintains a reference to an HDF5 identifier.
///
/// When you first create the identifier, use IdRef::claim() to grab it and make
/// sure it will be closed.
class IdRef final
{
public:
    /// Create an IdRef that takes ownership of an existing reference.
    ///
    /// This is intended for use when you receive an ID from the HDF5 library.
    ///
    /// The reference counter will be decremented on destruction, but will not
    /// be incremented.
    static IdRef claim(Id id);

    /// Create an IdRef that takes a new reference to the ID.
    ///
    /// The reference counter will be incremented on creation, and decremented
    /// on destruction.
    static IdRef ref(Id id);

    /// Create an IdRef that refers to a global ID.
    ///
    /// No reference counting will be done.
    static IdRef global(Id id)
    {
        return IdRef(id, true);
    }

    /// Create an IdRef that refers to a global ID. This call
    /// takes a non-global id and makes it global.
    ///
    /// No reference counting will be done.
    static IdRef global_ref(Id id);

    /// Create an invalid IdRef.
    ///
    /// The only use for the resulting object is to copy or move into it.
    IdRef() = default;

    IdRef(IdRef const& other);
    IdRef& operator=(IdRef const&);

    IdRef(IdRef && other)
        : m_id(other.m_id)
        , m_is_global_constant(other.m_is_global_constant)
    {
        other.m_id = -1;
        other.m_is_global_constant = false;
    }
    IdRef& operator=(IdRef && other)
    {
        std::swap(m_id, other.m_id);
        std::swap(m_is_global_constant, other.m_is_global_constant);
        return *this;
    }

    ~IdRef();

    void swap(IdRef & other) {
        std::swap(m_id, other.m_id);
        std::swap(m_is_global_constant, other.m_is_global_constant);
    }

    /// Take ownership of the ID.
    ///
    /// This object will no longer hold a reference to the ID. It is up to the
    /// called to deref the ID.
    Id release() {
        Id id = m_id;
        m_id = -1;
        m_is_global_constant = false;
        return id;
    }

    /// Get the ID.
    ///
    /// The reference count will not be changed. This is mostly for passing into
    /// HDF5 function calls.
    Id get() const
    {
        assert(m_id >= 0);
        return m_id;
    }

    /// Get the reference count of the ID.
    int ref_count() const;

    /// Check whether this IdRef contains an ID.
    ///
    /// Note that it does not check whether HDF5 thinks the ID is valid.
    explicit operator bool() const { return m_id >= 0; }

private:
    // use claim() or ref()
    explicit IdRef(Id id) : m_id(id), m_is_global_constant(false) {}
    explicit IdRef(Id id, bool is_global)
        : m_id(id)
        , m_is_global_constant(is_global)
    {}

    Id m_id = -1;
    bool m_is_global_constant = false;
};

}}