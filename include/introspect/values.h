#pragma once

#include "fwd.h"
#include <iostream>
#include <type_traits>
#include "inline_list.h"

INTROSPECT_NS_OPEN;

struct base_mirror
{
	virtual ~base_mirror() {}

	virtual size_t size() const = 0;
	virtual void *addr() = 0;
	const void *addr() const { return const_cast<base_mirror *>(this)->addr(); }

	virtual void parse(std::istream& str) = 0;
	virtual void print(std::ostream& str) const = 0;
};

inline std::istream& operator >> (std::istream& str, base_mirror& value)
{
	return value.parse(str), str;
}

inline std::ostream& operator << (std::ostream& str, const base_mirror& value)
{
	return value.print(str), str;
}

template<typename T, typename Base>
struct typed_mirror : Base
{
	explicit typed_mirror(T& raw) :
		raw(&raw)
	{
	}

	size_t size() const override { return sizeof(T); }
	void *addr() override { return raw; }
	T& get() { return *raw; }
	const T& get() const { return *raw; }

protected:
	T *raw;
};

template<typename T, typename Enable>
struct mirror : typed_mirror<T, base_mirror>
{
	explicit mirror(T& raw) :
		typed_mirror(raw) {}

	void parse(std::istream& str) override { str >> *raw; }
	void print(std::ostream& str) const override { str << *raw; }
};

// support enumerations

using enum_list = inline_list<enumerator>;

struct enumerator : enum_list::node
{
	const char * const name;
	const int64_t value;

	template<typename T>
	enumerator(const char *name, T value, enum_list& list):
		enum_list::node(list), name(name), value(value) {}
};

template<typename T>
struct enumerators : enum_list
{
	// specialize this template for your enum
	// and define enum_value's
};

#define INTROSPECT_ENUM(name) enumerator name##__enum__value__ { #name, name, *this }

struct base_enum : base_mirror
{
	virtual const enum_list& enums() const = 0;

protected:
	void parse(std::istream& str, int32_t *raw_value);
	void print(std::ostream& str, const int32_t *raw_value) const;
};

template<typename T>
struct enum_mirror : typed_mirror<T, base_enum>
{
	explicit enum_mirror(T& raw) :
		typed_mirror(raw) {}

	using E = typename std::underlying_type<T>::type;

	void parse(std::istream& str) override { base_enum::parse(str, reinterpret_cast<E *>(raw)); }
	void print(std::ostream& str) const override { base_enum::print(str, reinterpret_cast<E *>(raw)); }

	const enum_list& enums() const override {
		static enumerators<T> values;
		return values;
	}
};

template<typename T>
struct mirror<T, typename std::enable_if<std::is_enum<T>::value>::type>: enum_mirror<T>
{
	explicit mirror(T& raw) :
		enum_mirror(raw) {}
};


// value pointers

static constexpr size_t VARIANT_BUFFER_SIZE = sizeof(uintptr_t) * 4;

class variant : public base_mirror
{
public:

	template<typename T>
	variant(T&& init) {
		if (sizeof(T) <= VARIANT_BUFFER_SIZE)
			value = new (buffer) T(std::move(init));
		else
			value = new T(std::move(init));
	}

	variant(variant&& var)
	{
		if (var.is_inline()) {
			memcpy(buffer, var.buffer, VARIANT_BUFFER_SIZE);
			size_t offset = reinterpret_cast<uint8_t *>(var.value) - var.buffer;
			value = reinterpret_cast<base_mirror *>(buffer + offset);
		}
		else
			value = var.value;
		var.value = nullptr;
	}

	~variant() {
		if (value == nullptr)
			; // nothing to do
		else if (is_inline())
			value->~base_mirror();
		else
			delete value;
	}

	size_t size() const override { return value->size(); }
	void * addr() override { return value->addr(); }
	void parse(std::istream& str) override { value->parse(str); }
	void print(std::ostream& str) const override { value->print(str); }

protected:

	base_mirror *value;
	uint8_t buffer[VARIANT_BUFFER_SIZE];

	bool is_inline() const {
		auto addr = reinterpret_cast<uint8_t *>(value);
		return addr >= buffer && addr < buffer + VARIANT_BUFFER_SIZE;
	}
};

// TODO: support const variant
using const_variant = variant;

// arrays

struct base_array : base_mirror
{
	virtual size_t count() const = 0;

	virtual variant operator[](size_t i) = 0;
	const_variant operator[](size_t i) const { return const_cast<base_array *>(this)->operator[](i); }

	variant at(size_t i);
	const_variant at(size_t i) const { return const_cast<base_array *>(this)->at(i); }

	void parse(std::istream& str) override;
	void print(std::ostream& str) const override;
};

template<typename T>
struct typed_array : base_array
{
	explicit typed_array(T *raw, size_t len) :
		raw(raw), len(len) {} 

	size_t count() const override { return len; }
	size_t size() const override { return len * sizeof(T); }
	void * addr() override { return raw; }

	variant operator[](size_t i) override { 
		if (i >= len) {
			throw std::out_of_range("typed_array::at: out of range");
		}
		return mirror<T>(raw[i]);
	}

protected:
	T *		raw;
	size_t	len;
};

template<typename E, size_t N>
struct mirror<E[N]> : typed_array<E>
{
	using T = E[N];

	explicit mirror(T& raw) :
		typed_array(raw, N) {}

	T& get() { return *reinterpret_cast<T *>(raw); }
	const T& get() const { return *reinterpret_cast<T *>(raw); }
};

static_assert(sizeof(mirror<int>) <= VARIANT_BUFFER_SIZE, "VARIANT_BUFFER_SIZE is insufficient");
static_assert(sizeof(mirror<int[1]>) <= VARIANT_BUFFER_SIZE, "VARIANT_BUFFER_SIZE is insufficient");


INTROSPECT_NS_CLOSE;
