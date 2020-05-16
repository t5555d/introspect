#pragma once

#include "fields.h"

INTROSPECT_NS_OPEN;

//
// with_name attribute
//

struct with_name
{
public:
	with_name(const char* name) :
		m_name(name) {}

protected:
	void init(base_field* field) {
		field->m_name = m_name;
	}

private:
	const char* m_name;
};

//
// with_default attribute
//

struct has_default_value
{
	virtual void set_default() = 0;
};

template<typename T>
struct default_value : has_default_value
{
public:
	default_value(const T& value) :
		m_value(value) {}

	const T& get_default() const { return m_value; }

	void set_default() override
	{
		if (m_apply && m_field)
			(this->*m_apply)();
	}

protected:

	template<typename U, typename B>
	void init(typed_mirror<U, B>* field)
	{
		m_field = field;
		m_apply = &default_value::apply_single<U, B>;
	}

	template<typename U>
	void init(typed_array<U>* field)
	{
		m_field = field;
		m_apply = &default_value::apply_array<U>;
	}

private:

	template<typename U, typename B>
	void apply_single()
	{
		static_cast<typed_mirror<U, B>*>(m_field)->set(m_value);
	}

	template<typename U>
	void apply_array()
	{
		auto array = static_cast<typed_array<U>*>(m_field);
		for (size_t i = 0, n = array->count(); i < n; i++)
			array->set(i, m_value);
	}

	using apply_t = void (default_value::*)();

	T		m_value;
	void* m_field = nullptr;
	apply_t m_apply = nullptr;
};

template<typename T>
default_value<T> with_default(const T& value)
{
	return { value };
}

//
// with_min_count attribute
//

struct with_min_count
{
public:
	explicit with_min_count(size_t min_count) : m_min_count(min_count) {}
	size_t get_min_count() const { return m_min_count; }

protected:
	template<typename T>
	void init(typed_array<T>*) {}

private:
	size_t m_min_count;
};

INTROSPECT_NS_CLOSE;