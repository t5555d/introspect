#pragma once

#include "values.h"

INTROSPECT_NS_OPEN;

template<typename Struct, typename Fields>
struct struct_fields : Fields
{
	// specialize this template for your struct
	// and define STRUCT_FIELD's inside
};

// interface of Fields::field
#define STRUCT_FIELD(name) typename Fields::template field<decltype(raw->name)> name { #name, *raw, raw->name }

struct meta_field {
	ptrdiff_t offset;
};

template<typename Struct>
struct meta_fields
{
protected:

	static constexpr const Struct *raw = nullptr; // fake raw pointer

	template<typename Field>
	struct field : meta_field {
		field(const char *name, const base_fields& s, const base_field& f) :
			meta_field{ ptrdiff_t(&f) - ptrdiff_t(&s) }
		{
		}
	};
};

struct base_field
{
	base_field(const char *name, size_t offset, base_mirror& value) :
		name(name), offset(offset), value(value)
	{
	}
	virtual ~base_field() {}

	const char * const name;
	const size_t offset;
	base_mirror& value;
};

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

struct struct_mirror : base_mirror
{
	virtual base_fields& fields() = 0;

	const base_fields& fields() const {
		return const_cast<struct_mirror *>(this)->fields();
	}

    VISIT_IMPL;
};

template<typename Struct>
struct real_fields : base_fields
{
	template <typename E>
	struct field : public base_field
	{
		typename mirror<E> value;

		field(const char *name, const Struct& str, const E& raw) :
			base_field(name, uintptr_t(&raw) - uintptr_t(&str), value)
		{
		}
	};

	void set_fields(Struct& value) {
        auto base = reinterpret_cast<uint8_t *>(&value);
        for (auto& field : *this) {
            field.value.addr(base + field.offset);
        }
	}

    const char *type() const override { return typeid(Struct).name(); }

protected:

	using child_fields = struct_fields<Struct, real_fields<Struct>>;

	array_ptr<const meta_field> meta() const override {
		// describe fields of our derivative:
		static struct_fields<Struct, meta_fields<child_fields> > x;
		return array_cast<const meta_field>(x);
	}

    static constexpr const Struct *raw = nullptr; // fake raw pointer
};

template<typename Struct>
struct mirror<Struct, typename std::enable_if<std::is_class<Struct>::value>::type> :
	typed_mirror<Struct, struct_mirror>,
	struct_fields<Struct, real_fields<Struct>>
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
