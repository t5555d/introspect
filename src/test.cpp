#include <iostream>
#include <sstream>
#include <utility>
#include <stdint.h>
#include "introspect/fields.h"
#include "introspect/io.h"

using namespace introspect;

// simple settings

constexpr int DEFAULT_VALUE = 0xAE;

enum enum_t
{
	VALUE0,
	VALUE1,
};

struct point_t
{
    int32_t x;
    int32_t y;
    int32_t z;
};

struct settings_t
{
	int32_t a;
	int64_t b;
	enum_t  c;
	int32_t d[3];
    point_t e;
};

void set_defaults(settings_t *set)
{
	memset(set, DEFAULT_VALUE, sizeof(settings_t));
}

template<>
struct enum_values<enum_t>
{
	ENUM_VALUE(VALUE0);
	ENUM_VALUE(VALUE1);
};

template<typename Fields>
struct struct_fields<point_t, Fields> : Fields
{
    STRUCT_FIELD(x, with_name("X"));
    STRUCT_FIELD(y, with_name("Y"));
    STRUCT_FIELD(z, with_name("Z"));
};

template<typename Fields>
struct struct_fields<settings_t, Fields> : Fields
{
	STRUCT_FIELD(a);
	STRUCT_FIELD(b);
	STRUCT_FIELD(c);
	STRUCT_FIELD(d);
    STRUCT_FIELD(e);
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
    settings.e.x = 35;
    settings.e.y = 40;
    settings.e.z = 45;

    try {
        settings_c set(settings);

        std::cout << "Accessing raw values: " << std::endl;
        std::cout << set.a.name() << " = " << set.a.value.get() << std::endl;
        std::cout << set.b.name() << " = " << set.b.value.get() << std::endl;
        std::cout << set.c.name() << " = " << set.c.value.get() << std::endl;
        std::cout << set.d.name() << " = " << set.d.value.get() << std::endl;
        std::cout << set.d.name() << " = "
            << "{ " << set.d.value.get()[0]
            << ", " << set.d.value.get()[1]
            << ", " << set.d.value.get()[2]
            << " }" << std::endl;
        std::cout << set.e.name() << ".x = " << set.e.value.get().x << std::endl;
        std::cout << set.e.name() << ".y = " << set.e.value.get().y << std::endl;
        std::cout << set.e.name() << ".z = " << set.e.value.get().z << std::endl;

        std::cout << "Accessing mirrors: " << std::endl;
        std::cout << set.a.name() << " = " << set.a.value << std::endl;
        std::cout << set.b.name() << " = " << set.b.value << std::endl;
        std::cout << set.c.name() << " = " << set.c.value << std::endl;
        std::cout << set.d.name() << " = " << set.d.value << std::endl;
        std::cout << set.d.name() << " = "
            << "{ " << set.d.value[0]
            << ", " << set.d.value[1]
            << ", " << set.d.value[2]
            << " }" << std::endl;

        std::cout << set.e.name() << "." << set.e.value.x.name() << " = " << set.e.value.x.value << std::endl;
        std::cout << set.e.name() << "." << set.e.value.y.name() << " = " << set.e.value.y.value << std::endl;
        std::cout << set.e.name() << "." << set.e.value.z.name() << " = " << set.e.value.z.value << std::endl;

        std::cout << "Print the whole struct: \n" << set;

        std::stringstream buffer;
        buffer << set;
        set_defaults(&settings);
        std::cout << "After set_defaults: \n" << set;

        while (!buffer.eof())
            buffer >> set;
        std::cout << "After parsing: \n" << set;

    }
    catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
    }

}