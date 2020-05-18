#pragma once

#include "values.h"

INTROSPECT_NS_OPEN;

// specialize this template for your struct
// and define STRUCT_FIELD's inside
template<typename Struct, typename Fields>
struct struct_fields;

#define STRUCT_FIELDS(name) template<typename Fields> \
	struct struct_fields<name, Fields> : Fields::template base<name>

// interface of Fields::fields<Struct>
// - static const Struct *raw; // usually fake pointer
// - auto create_field(const char *name, Struct *raw, T *field, ...)
// - it should be iterable (begin() / end())

#define STRUCT_FIELD2(name, type, ...) \
	decltype(create_field(#name, raw, (const field_type_t<type> *) nullptr, __VA_ARGS__)) \
	  name = create_field(#name, raw, &raw->name, __VA_ARGS__);

#define STRUCT_FIELD(name, ...) STRUCT_FIELD2(name, decltype(raw->name), __VA_ARGS__)

#define FAKE_RAW_PTR ((uintptr_t)0xDEAD0000)

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


struct base_field : virtual base_mirror
{
	base_field(const char *name, ptrdiff_t offset) :
		offset{ offset }, m_name(name)
	{
	}
	virtual ~base_field() {}

	const char *name() const { return m_name; }

public:
	const ptrdiff_t offset;

private:
	friend struct with_name;
	const char *m_name;
};

//
// field_offsets
// utility template for iteration over fields
// its create_field returns `field_offset` of constant size
// and hence struct_fields<Struct, field_offset<Fields, Field>> 
// can be reinterpret_cast to meta_field[N]
//

struct field_offset
{
	ptrdiff_t value;
	static const ptrdiff_t INVALID_VALUE = std::numeric_limits<ptrdiff_t>::min();
};

template<typename Fields, typename Field>
struct field_offsets
{
	template<typename Struct>
	struct base
	{
	protected:
		using base_t = typename Fields::template base<Struct>;

		static constexpr auto raw = (const struct_fields<Struct, Fields>*)FAKE_RAW_PTR;

		static field_offset create_field(const char* name, const base_t* b, const Field* f, ...)
		{
			auto offset = ptrdiff_t(f) - ptrdiff_t(b);
			return { offset };
		}

		// fallback for fields, that don't match Field
		static field_offset create_field(const char* name, ...)
		{
			return { field_offset::INVALID_VALUE };
		}
	};
};

//
// iterable field offsets:
//

template<typename Struct, typename Fields, typename Field>
struct field_offset_set
{
	field_offset_set() {

		// filter and count offsets:
		auto array = reinterpret_cast<field_offset*>(&m_offsets);
		constexpr size_t max_size = sizeof(m_offsets) / sizeof(field_offset);
		m_size = 0;
		for (size_t i = 0; i < max_size; i++) {
			if (array[i].value != field_offset::INVALID_VALUE) {
				if (m_size < i)
					array[m_size] = array[i];
				m_size++;
			}
		}
	}

	const field_offset* begin() const {
		return reinterpret_cast<const field_offset*>(&m_offsets);
	}

	const field_offset* end() const {
		return begin() + m_size;
	}

	size_t size() const { return m_size; }

private:
	struct_fields<Struct, field_offsets<Fields, Field>> m_offsets;
	size_t m_size;
};

//
// field_set : helper class for iteration over fields
//

template<typename Field>
struct field_set
{
	struct iterator
	{
		typedef Field value_type;
		typedef Field* pointer;
		typedef Field& reference;

		iterator& operator++() {
			m_offset++;
			return *this;
		}

		iterator& operator--() {
			m_offset--;
			return *this;
		}

		iterator operator++(int) { return{ m_base, m_offset + 1 }; }
		iterator operator--(int) { return{ m_base, m_offset - 1 }; }

		pointer operator->() const { return reinterpret_cast<pointer>(m_base + m_offset->value); }
		reference operator*() const { return *operator->(); }

		bool operator==(const iterator& that) const { return m_base == that.m_base && m_offset == that.m_offset; }
		bool operator!=(const iterator& that) const { return !(*this == that); }

	private:
		friend struct field_set;

		iterator(ptrdiff_t m_base, const field_offset* m_offset) :
			m_base(m_base), m_offset(m_offset) {}

		ptrdiff_t m_base;
		const field_offset* m_offset;
	};

	iterator begin() { return{ m_base, m_beg }; }
	iterator end() { return{ m_base, m_end }; }

	field_set(ptrdiff_t base, const field_offset* beg, const field_offset* end) :
		m_base(base), m_beg(beg), m_end(end) {}

	template<typename Struct, typename Fields>
	field_set(typename Fields::template base<Struct>* base, const field_offset_set<Struct, Fields, Field>& offsets):
		field_set(ptrdiff_t(base), offsets.begin(), offsets.end()) {}

	operator field_set<const Field>() {
		return { m_base, m_beg, m_end };
	}

private:
	ptrdiff_t m_base;
	const field_offset* m_beg;
	const field_offset* m_end;
};

// is_struct helper

template<typename T>
struct is_struct
{
	enum { value = std::is_class<T>::value };
};

template<typename T, size_t N>
struct is_struct<std::array<T, N>>
{
	enum { value = false };
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
		mirror<T, typename std::conditional<is_struct<T>::value, simple_fields, void>::type>,
		Args...
	{
		field(const char* name, ptrdiff_t offset, Args... args) :
			base_field(name, offset), Args(args)...
		{
			int dummy[]{ 0, (Args::init(this), 0)... };
		}
	};

	template<typename Struct>
	struct base
	{
		void set_fields(Struct& value) {
			auto m_base = reinterpret_cast<uint8_t*>(&value);
			for (auto& field : fields<base_field>()) {
				field.addr(m_base + field.offset);
			}
		}

		template<typename Field>
		field_set<Field> fields()
		{
			static field_offset_set<Struct, simple_fields, Field> offsets;
			return field_set<Field>(this, offsets);
		}

		template<typename Field>
		field_set<const Field> fields() const
		{
			return const_cast<base*>(this)->fields<Field>();
		}

	protected:
		static constexpr auto raw = (const Struct*)FAKE_RAW_PTR;

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
	struct base {
	protected:
		static constexpr auto raw = (const Struct*)FAKE_RAW_PTR;

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
		static std::array<T, N> create_field(const char* name, const Struct* str, const std::array<T, N>* field, const filler<U>& value, ...)
		{
			std::array<T, N> array;
			array.fill(value.get_filler());
			return array;
		}

	};
};

struct struct_mirror : virtual base_mirror
{
	virtual field_set<base_field> fields() = 0;

	field_set<const base_field> fields() const {
		return const_cast<struct_mirror*>(this)->fields();
	}

	base_field& at(const char* name);

	const base_field& at(const char* name) const {
		return const_cast<struct_mirror*>(this)->at(name);
	}

	base_field& operator[](const char* name) { return at(name); }
	const base_field& operator[](const char* name) const { return at(name); }

	VISIT_IMPL;
};

template<typename Struct, typename Fields>
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

	using struct_fields<Struct, Fields>::fields;
	field_set<base_field> fields() override { return fields<base_field>(); }

    void addr(void *addr) override {
        typed_mirror::addr(addr);
        set_fields(get());
    }
};

INTROSPECT_NS_CLOSE;
