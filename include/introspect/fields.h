#pragma once

#include "inline_list.h"
#include "values.h"

INTROSPECT_NS_OPEN;

struct base_struct : 
	base_mirror,
	inline_list<base_field>
{
	base_field& at(const char *name);

	const base_field& at(const char *name) const {
		return const_cast<base_struct *>(this)->at(name);
	}

	base_field& operator[](const char *name) { return at(name); }
	const base_field& operator[](const char *name) const { return at(name); }

	void parse(std::istream& str) override;
	void print(std::ostream& str) const override;
};

struct base_field : base_struct::node
{
	using node = base_struct::node;

	base_field(const char *name, size_t offset, base_mirror& value, base_struct& fields) :
		base_struct::node(fields), name(name), offset(offset), value(value)
	{
	}
	virtual ~base_field() {}

	const char * const name;
	const size_t offset;
	base_mirror& value;
};

template<typename Struct>
struct struct_mirror : typed_mirror<Struct, base_struct>
{
	explicit struct_mirror(Struct& raw) :
		typed_mirror(raw) {}

	template <typename T>
	struct field : public base_field
	{
		typename mirror<T> value;

		field(const char *name, Struct& str, T& raw, base_struct& fields) :
			base_field(name, uintptr_t(&raw) - uintptr_t(&str), value, fields),
			value(raw)
		{
		}
	};
};

#define INTROSPECT(name) field<decltype(raw->name)> name { #name, *raw, raw->name, *this }

INTROSPECT_NS_CLOSE;
