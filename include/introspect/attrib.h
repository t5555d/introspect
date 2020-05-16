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

	template<typename U>
	void init(mirror<U>* field)
	{
		m_field = field;
		m_apply = &default_value::apply<U>;
	}

private:

	template<typename U>
	void apply()
	{
		static_cast<mirror<U>*>(m_field)->set(m_value);
	}

	using apply_t = void (default_value::*)();

	const T	m_value;
	void*	m_field = nullptr;
	apply_t m_apply = nullptr;
};

template<typename T>
default_value<T> with_default(const T& value)
{
	return { value };
}

//
// with_filler attribute
//

struct has_filler : has_default_value
{
	virtual void fill(size_t i) = 0;
	void fill(size_t i, size_t j) {
		while (i < j) fill(i++);
	}
};

template<typename T>
struct filler : has_filler
{
	filler(const T& value) : 
		m_value(value) {}

	void fill(size_t i) override
	{
		if (m_apply && m_array)
			(this->*m_apply)(i);
	}

	void set_default() override
	{
		if (m_apply && m_array) {
			for (size_t i = 0, n = m_array->count(); i < n; i++)
				(this->*m_apply)(i);
		}
	}

protected:
	template<typename U>
	void init(typed_array<U>* field)
	{
		m_array = field;
		m_apply = &filler::apply<U>;
	}

private:

	template<typename U>
	void apply(size_t i)
	{
		static_cast<typed_array<U>*>(m_array)->set(i, m_value);
	}

	using apply_t = void (filler::*)(size_t i);

	const T			m_value;
	array_mirror*   m_array = nullptr;
	apply_t			m_apply = nullptr;
};

template<typename T>
filler<T> with_filler(const T& value)
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

	has_filler* get_filler() { return m_filler; }

protected:
	void init(has_filler* filler) { m_filler = filler; }

private:
	size_t m_min_count;
	has_filler* m_filler = nullptr;
};

INTROSPECT_NS_CLOSE;