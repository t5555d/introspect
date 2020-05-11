#pragma once

#include "values.h"

INTROSPECT_NS_OPEN;

template<typename Struct, typename Fields>
struct struct_fields : Fields::template fields<Struct>
{
	// specialize this template for your struct
	// and define STRUCT_FIELD's inside
};

// interface of Fields::fields<Struct>
// - static const Struct *raw; // usually fake pointer
// - auto create_field(const char *name, Struct *raw, T *field, ...)
// - it should be iterable (begin() / end())
#define STRUCT_FIELD(name, ...) decltype(create_field(#name, raw, &raw->name)) name = create_field(#name, raw, &raw->name, __VA_ARGS__)

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

struct with_name
{
	explicit with_name(const char* name) : name(name) {}
	const char* name;

	void apply(base_field& field) {
		field.m_name = name;
	}
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
	};

};

//
// simple fields template
// support for typed fields 
// and some basic field attributes
//

struct simple_fields
{
	template <typename T>
	struct field : base_field, mirror<T>
	{
		field(const char* name, ptrdiff_t offset) :
			base_field(name, offset) {}
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
		static field<T> create_field(const char* name, const Struct* s, const T* f, Args... args)
		{
			field<T> field(name, ptrdiff_t(f) - ptrdiff_t(s));
			int dummy[]{ 0, (args.apply(field), 0)... };
			return field;
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

template<typename Struct>
struct mirror<Struct, typename std::enable_if<std::is_class<Struct>::value>::type> :
	typed_mirror<Struct, struct_mirror>,
	struct_fields<Struct, simple_fields>
{
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
