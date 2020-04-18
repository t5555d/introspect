#pragma once

#include "fwd.h"
#include "errors.h"
#include <type_traits>
#include "utils.h"

INTROSPECT_NS_OPEN;

struct visitor
{
    virtual void visit(base_mirror& value);
    virtual void visit(int_mirror& value);
    virtual void visit(enum_mirror& value);
    virtual void visit(float_mirror& value);
    virtual void visit(array_mirror& value);
    virtual void visit(struct_mirror& value);
};

struct const_visitor
{
    virtual void visit(const base_mirror& value);
    virtual void visit(const int_mirror& value);
    virtual void visit(const enum_mirror& value);
    virtual void visit(const float_mirror& value);
    virtual void visit(const array_mirror& value);
    virtual void visit(const struct_mirror& value);
};

#define VISIT_IMPL \
    virtual void visit(visitor& v) { v.visit(*this); } \
    virtual void visit(const_visitor& v) const { v.visit(*this); }

struct base_mirror
{
	virtual ~base_mirror() {}

	virtual size_t size() const = 0;
	virtual void *addr() = 0;
	const void *addr() const { return const_cast<base_mirror *>(this)->addr(); }
    virtual void addr(void *addr) = 0;
    virtual const char *type() const = 0;

    VISIT_IMPL
};

template<typename T, typename Base>
struct typed_mirror : Base
{
    typed_mirror() = default;
    typed_mirror(const typed_mirror& that) = default;
    explicit typed_mirror(T& raw) : raw(&raw) {}

    size_t size() const override { return sizeof(T); }
    void *addr() override { return raw; }
    void addr(void *addr) override { raw = reinterpret_cast<T *>(addr); }
    const char *type() const override { return typeid(T).name(); }

    T& get() { return *raw; }
    const T& get() const { return *raw; }
    void set(T& raw) { addr(&raw); }

protected:
    T *raw = nullptr;
};

template<typename T, typename Enable>
struct mirror : typed_mirror<T, base_mirror>
{
    mirror() = default;
    mirror(const mirror& that) = default;
    explicit mirror(T& raw) : typed_mirror(raw) {}
};

//
// arithmetic types
//

struct int_mirror : base_mirror
{
    VISIT_IMPL;

    virtual int64_t int_value() const = 0;
    virtual void int_value(int64_t value) = 0;
};

template<typename T>
struct mirror<T, typename std::enable_if<std::is_integral<T>::value>::type> :
    typed_mirror<T, int_mirror>
{
    mirror() = default;
    mirror(const mirror& that) = default;
    explicit mirror(T& raw) : typed_mirror(raw) {}

    int64_t int_value() const override { return *raw; }
    void int_value(int64_t value) override { *raw = static_cast<T>(value); }
};

struct float_mirror : base_mirror
{
    VISIT_IMPL;

    virtual double float_value() const = 0;
    virtual void float_value(double value) = 0;
};

template<typename T>
struct mirror<T, typename std::enable_if<std::is_floating_point<T>::value>::type> :
    typed_mirror<T, float_mirror>
{
    mirror() = default;
    mirror(const mirror& that) = default;
    explicit mirror(T& raw) : typed_mirror(raw) {}

    double float_value() const override { return *raw; }
    void float_value(double value) override { *raw = static_cast<T>(value); }
};

// support enumerations

struct enum_value
{
	const char * name;
	int64_t value;
};

template<typename T>
struct enum_values
{
	// specialize this template for your enum
	// and define enum_value's inside
};

#define ENUM_VALUE(name) enum_value __enum__value__##name { #name, name }

struct enum_mirror : int_mirror
{
    VISIT_IMPL;

    virtual array_ptr<const enum_value> values() const = 0;
};

template<typename T>
struct typed_enum : typed_mirror<T, enum_mirror>
{
	typed_enum() = default;
	typed_enum(const typed_enum& that) = default;
	explicit typed_enum(T& raw) : typed_mirror(raw) {}

	using E = typename std::underlying_type<T>::type;

    int64_t int_value() const override { return *raw; }
    void int_value(int64_t value) override { *raw = static_cast<T>(value); }

	array_ptr<const enum_value> values() const override {
		static enum_values<T> x;
		return array_cast<enum_value>(x);
	}
};

template<typename T>
struct mirror<T, typename std::enable_if<std::is_enum<T>::value>::type>: typed_enum<T>
{
    mirror() = default;
    mirror(const mirror& that) = default;
	explicit mirror(T& raw) : typed_enum(raw) {}
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

    variant(variant&& var);
    ~variant();

	size_t size() const override { return value->size(); }
	void * addr() override { return value->addr(); }
    void addr(void *addr) override { value->addr(addr); }
    const char *type() const override { return value->type(); }

    void visit(visitor& v) override { value->visit(v); }
	void visit(const_visitor& v) const override { value->visit(v); }

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

struct array_mirror : base_mirror
{
    virtual size_t count() const = 0;

	virtual variant operator[](size_t i) = 0;
	const_variant operator[](size_t i) const { return const_cast<array_mirror *>(this)->operator[](i); }

	variant at(size_t i);
	const_variant at(size_t i) const { return const_cast<array_mirror *>(this)->at(i); }

    VISIT_IMPL;
};

template<typename T>
struct typed_array : array_mirror
{
    typed_array(const typed_array& that) = default;
    explicit typed_array(T *raw, size_t len) :
		raw(raw), len(len) {} 

	size_t count() const override { return len; }
	size_t size() const override { return len * sizeof(T); }
	void * addr() override { return raw; }
    void addr(void *addr) override { raw = reinterpret_cast<T *>(addr); }

	variant operator[](size_t i) override { 
        if (i >= len)
            throw bad_idx_error(i, len);
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

    mirror() : typed_array(nullptr, N) {}
    mirror(const mirror& that) = default;
	explicit mirror(T& raw) :
		typed_array(raw, N) {}

	T& get() { return *reinterpret_cast<T *>(raw); }
	const T& get() const { return *reinterpret_cast<T *>(raw); }
    const char *type() const override { return typeid(T).name(); }

	void set(T& value) { raw = &value[0]; }
};

static_assert(sizeof(mirror<int>) <= VARIANT_BUFFER_SIZE, "VARIANT_BUFFER_SIZE is insufficient");
static_assert(sizeof(mirror<int[1]>) <= VARIANT_BUFFER_SIZE, "VARIANT_BUFFER_SIZE is insufficient");


INTROSPECT_NS_CLOSE;
