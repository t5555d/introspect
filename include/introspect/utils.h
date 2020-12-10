#pragma once

#include "fwd.h"

INTROSPECT_NS_OPEN;

template<typename E>
class array_ptr
{
    E *     m_base;
    size_t  m_size;
public:

    array_ptr(E *base, size_t *size) :
        m_base(base), m_size(size) {}

    template<size_t N>
    array_ptr(E(&raw)[N]) :
        m_base(raw), m_size(N) {}

    size_t size() const { return m_size; }
    E *begin() { return m_base; }
    E *end() { return m_base + m_size; }
    const E *begin() const { return m_base; }
    const E *end() const { return m_base + m_size; }
};

template<typename E, typename S>
using as_array = E[sizeof(S) / sizeof(E)];

template<typename E, typename S>
as_array<E, S>& array_cast(S& value)
{
    return reinterpret_cast<as_array<E, S>&>(value);
}

INTROSPECT_NS_CLOSE;
