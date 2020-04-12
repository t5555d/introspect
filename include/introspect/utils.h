#pragma once

#include "fwd.h"

INTROSPECT_NS_OPEN;

template<typename E>
class array_ptr
{
	E *		m_base;
	size_t	m_size;
public:

	array_ptr(E *base, size_t *size) :
		m_base(base), m_size(size) {}

	size_t size() const { return m_size; }
	E *begin() { return m_base; }
	E *end() { return m_base + m_size; }
	const E *begin() const { return m_base; }
	const E *end() const { return m_base + m_size; }
};

template<typename S, typename E>
class array_cast : public array_ptr<E>
{
	S value;
public:
	wrap_array():
		array_ptr(reinterpret_cast<E *>(&value), sizeof(S) / sizeof(E)) 
	{
	}
};

INTROSPECT_NS_CLOSE;
