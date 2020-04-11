#include <iostream>
#include <utility>
#include <stdint.h>
#include "introspect/fields.h"

using namespace introspect;

// simple settings

constexpr int DEFAULT_VALUE = 0xAE;

enum enum_t
{
	VALUE0,
	VALUE1,
};

struct settings_t
{
	int32_t a;
	int64_t b;
	enum_t  c;
	int32_t d[3];
};

void set_defaults(settings_t *set)
{
	memset(set, DEFAULT_VALUE, sizeof(settings_t));
}

template<>
struct enum_values<enum_t> : base_enum_values
{
	enum_values() : base_enum_values(this) {}

	ENUM_VALUE(VALUE0);
	ENUM_VALUE(VALUE1);
};

template<>
struct mirror<settings_t> : struct_mirror<settings_t>
{
	explicit mirror(settings_t& set) :
		struct_mirror{ set } {}

	INTROSPECT(a);
	INTROSPECT(b);
	INTROSPECT(c);
	INTROSPECT(d);
};

// generic introspective settings

using settings_c = mirror<settings_t>;

int main()
{
	settings_t settings;

	set_defaults(&settings);

	settings.a = 5;
	settings.b = 10;
	settings.c = VALUE1;
	settings.d[0] = 20;
	settings.d[1] = 25;
	settings.d[2] = 30;

	settings_c set(settings);

	std::cout << "Accessing raw values: " << std::endl;
	std::cout << set.a.name << " = " << set.a.value.get() << std::endl;
	std::cout << set.b.name << " = " << set.b.value.get() << std::endl;
	std::cout << set.c.name << " = " << set.c.value.get() << std::endl;
	std::cout << set.d.name << " = " << set.d.value.get() << std::endl;
	std::cout << set.d.name << " = "
		<< "{ " << set.d.value.get()[0]
		<< ", " << set.d.value.get()[1]
		<< ", " << set.d.value.get()[2]
		<< " }" << std::endl;

	std::cout << "Accessing mirrors: " << std::endl;
	std::cout << set.a.name << " = " << set.a.value << std::endl;
	std::cout << set.b.name << " = " << set.b.value << std::endl;
	std::cout << set.c.name << " = " << set.c.value << std::endl;
	std::cout << set.d.name << " = " << set.d.value << std::endl;
	std::cout << set.d.name << " = "
		<< "{ " << set.d.value[0]
		<< ", " << set.d.value[1]
		<< ", " << set.d.value[2]
		<< " }" << std::endl;

	std::cout << "Print the whole struct: \n" << set;
}