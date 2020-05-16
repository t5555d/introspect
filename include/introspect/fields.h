#pragma once

#include "values.h"

INTROSPECT_NS_OPEN;

// specialize this template for your struct
// and define STRUCT_FIELD's inside
template<typename Struct, typename Fields>
struct struct_fields;

#define STRUCT_FIELDS(name) template<typename Fields> \
	struct struct_fields<name, Fields> : Fields::template fields<name>

// interface of Fields::fields<Struct>
// - static const Struct *raw; // usually fake pointer
// - auto create_field(const char *name, Struct *raw, T *field, ...)
// - it should be iterable (begin() / end())

#define STRUCT_FIELD2(name, type, ...) \
	decltype(create_field(#name, raw, (const field_type_t<type> *) nullptr, __VA_ARGS__)) \
	  name = create_field(#name, raw, &raw->name, __VA_ARGS__);

#define STRUCT_FIELD(name, ...) STRUCT_FIELD2(name, decltype(raw->name), __VA_ARGS__)

//
// field type mapping
//

// default type mapping
template<typename T>
struct field_type
{
	using type = T;
};

// type mapping for array
template<typename T, size_t N>
struct field_type<T[N]>
{
	using type = std::array<T, N>;
};

template<typename T>
using field_type_t = typename field_type<T>::type;


struct meta_field
{
	const ptrdiff_t offset;
};

struct base_field : meta_field, virtual base_mirror
{
	base_field(const char *name, ptrdiff_t offset) :
		meta_field{ offset }, m_name(name)
	{
	}
	virtual ~base_field() {}

	const char *name() const { return m_name; }

private:
	friend struct with_name;
	const char *m_name;
};

//
// base_fields : base class for Fields::fields
// provide means for iteration over base_field's
//

struct base_fields
{
	base_field& at(const char *name);

	const base_field& at(const char *name) const {
		return const_cast<base_fields *>(this)->at(name);
	}

	base_field& operator[](const char *name) { return at(name); }
	const base_field& operator[](const char *name) const { return at(name); }

	template<typename Fields, typename Field>
	struct basic_iterator
	{
		typedef Field value_type;
		typedef Field* pointer;
		typedef Field& reference;

		basic_iterator& operator++() {
			pfield++;
			return *this;
		}

		basic_iterator& operator--() {
			pfield--;
			return *this;
		}

		basic_iterator operator++(int) { return{ fields, pfield + 1 }; }
		basic_iterator operator--(int) { return{ fields, pfield - 1 }; }

		reference operator*() const { return *operator->(); }
		pointer operator->() const { return reinterpret_cast<pointer>(ptrdiff_t(fields) + pfield->offset); }

		bool operator==(const basic_iterator& that) const { return fields == that.fields && pfield == that.pfield; }
		bool operator!=(const basic_iterator& that) const { return !(*this == that); }

	private:
		friend struct base_fields;

		basic_iterator(Fields *fields, const meta_field *pfield) :
			fields(fields), pfield(pfield) {}

		Fields *fields;
		const meta_field *pfield;
	};

	using iterator = basic_iterator<base_fields, base_field>;
	using const_iterator = basic_iterator<const base_fields, const base_field>;

	iterator begin() { return{ this, meta().begin() }; }
	iterator end() { return{ this, meta().end() }; }
	const_iterator begin() const { return{ this, meta().begin() }; }
	const_iterator end() const { return{ this, meta().end() }; }

protected:
	virtual array_ptr<const meta_field> meta() const = 0;
    virtual const char *type() const = 0;
};

//
// meta_fields
// utility template for iteration over base_field's
// its create_field returns `meta_field` of constant size
// and hence struct_fields<Struct, meta_fields<Fields>> 
// can be reinterpret_cast to meta_field[N]
//

template<typename Fields>
struct meta_fields
{
	using field = meta_field;

	template<typename Struct>
	struct fields
	{
	protected:

		static constexpr const struct_fields<Struct, Fields>* raw = nullptr;

		static meta_field create_field(const char* name, const base_fields* s, const base_field* f, ...)
		{
			auto offset = ptrdiff_t(f) - ptrdiff_t(s);
			return { offset };
		}

		// support for STRUCT_FIELD2: should never be called
		static meta_field create_field(const char* name, const base_fields* raw, const void* field, ...);
	};

};

//
// simple fields template
// support for typed fields 
// and some basic field attributes
//

struct simple_fields
{
	template <typename T, typename... Args>
	struct field : 
		base_field, 
		mirror<T, typename std::conditional<std::is_class<T>::value, simple_fields, void>::type>,
		Args...
	{
		field(const char* name, ptrdiff_t offset, Args... args) :
			base_field(name, offset), Args(args)...
		{
			int dummy[]{ 0, (Args::init(this), 0)... };
		}
	};

	template<typename Struct>
	struct fields : base_fields
	{
		void set_fields(Struct& value) {
			auto base = reinterpret_cast<uint8_t*>(&value);
			for (auto& field : *this) {
				field.addr(base + field.offset);
			}
		}

		const char* type() const override { return typeid(Struct).name(); }

	protected:

		array_ptr<const meta_field> meta() const override {
			// describe fields of our derivative:
			static struct_fields<Struct, meta_fields<simple_fields> > x;
			return array_cast<const meta_field>(x);
		}

		static constexpr const Struct* raw = nullptr; // fake raw pointer

		template<typename T, typename... Args>
		static field<T, Args...> create_field(const char* name, const Struct* s, const T* f, Args... args)
		{
			return field<T, Args...>(name, ptrdiff_t(f) - ptrdiff_t(s), args...);
		}
	};
};

struct raw_fields
{
	template<typename Struct>
	struct fields {
	protected:
		static constexpr const Struct* raw = nullptr;

		template<typename T>
		static T create_field(const char* name, const Struct* str, const T* field, ...)
		{
			return T{};
		}

		template<typename T, typename U>
		static T create_field(const char* name, const Struct* str, const T* field, const default_value<U>& value, ...)
		{
			return value.get_default();
		}

		template<typename T, size_t N, typename U>
		static std::array<T, N> create_field(const char* name, const Struct* str, const std::array<T, N>* field, const default_value<U>& value, ...)
		{
			std::array<T, N> array;
			array.fill(value.get_default());
			return array;
		}

	};
};

struct struct_mirror : virtual base_mirror
{
	virtual base_fields& fields() = 0;

	const base_fields& fields() const {
		return const_cast<struct_mirror*>(this)->fields();
	}

	VISIT_IMPL;
};

template<typename Struct, typename Fields, typename Enable>
struct mirror :
	typed_mirror<Struct, struct_mirror>,
	struct_fields<Struct, Fields>
{
	static_assert(std::is_class<Struct>::value, "Define partial specialization for your type");

	mirror() = default;
	mirror(const mirror& that) = default;
	explicit mirror(Struct& raw) : typed_mirror(raw)
    {
        set_fields(raw);
    }

	base_fields& fields() override { return *this; }

    void addr(void *addr) override {
        typed_mirror::addr(addr);
        set_fields(get());
    }
};

INTROSPECT_NS_CLOSE;
