#include <iostream>
#include <sstream>
#include <utility>
#include <stdint.h>
#include "introspect/fields.h"
#include "introspect/attrib.h"
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

ENUM_OPTIONS(enum_t)
{
    ENUM_OPTION(VALUE0);
    ENUM_OPTION(VALUE1);
};

STRUCT_FIELDS(point_t)
{
    STRUCT_FIELD(x, with_name("X"));
    STRUCT_FIELD(y, with_name("Y"));
    STRUCT_FIELD(z, with_name("Z"));
};

struct other_settings_t
{
    int32_t a[3];
    int32_t b;
    int32_t c;
    double d;
    int32_t e;
    double f;
    int32_t i;
    int64_t j;
    point_t s;
};

struct settings_t;

STRUCT_FIELDS(settings_t)
{
    STRUCT_FIELD2(a, int[3],    with_filler(-1),        maps_to(&other_settings_t::a), with_min_count(1));
    STRUCT_FIELD2(b, bool,      with_default(0),        maps_to(&other_settings_t::b));
    STRUCT_FIELD2(c, char,      with_default(0),        maps_to(&other_settings_t::c));
    STRUCT_FIELD2(d, double,    with_default(0),        maps_to(&other_settings_t::d));
    STRUCT_FIELD2(e, enum_t,    with_default(VALUE0),   maps_to(&other_settings_t::e));
    STRUCT_FIELD2(f, float,     with_default(0.0f),     maps_to(&other_settings_t::f));
    STRUCT_FIELD2(i, int,       with_default(0),        maps_to(&other_settings_t::i));
    STRUCT_FIELD2(j, int64_t,   with_default(0),        maps_to(&other_settings_t::j));
    STRUCT_FIELD2(s, point_t,   maps_to(&other_settings_t::s));
};

struct settings_t : struct_fields<settings_t, raw_fields> {};

void set_defaults(settings_t* set)
{
    memset(set, DEFAULT_VALUE, sizeof(settings_t));
}

// generic introspective settings

using settings_c = mirror<settings_t, simple_fields>;

template<typename Field, typename Fields>
void list_fields(const Fields& m)
{
    std::cout << "Fields of type " << typeid(Field).name() << ":";
    for (auto& field : m.fields<Field>())
        std::cout << " " << dynamic_cast<const base_field&>(field).name();
    std::cout << std::endl;
}

int main()
{
	settings_t settings;

	set_defaults(&settings);

    settings.a[0] = 1;
    settings.a[1] = 2;
    settings.a[2] = 3;
    settings.b = true;
    settings.c = 'x';
    settings.d = 4.5;
    settings.e = VALUE1;
    settings.f = 6.7f;
    settings.i = 8;
	settings.j = 9;
    settings.s.x = 10;
    settings.s.y = 11;
    settings.s.z = 12;

    try {
        settings_c set(settings);

        std::cout << "Accessing raw values: " << std::endl;
        std::cout << set.a.name() << " = " << set.a.get() << std::endl;
        std::cout << set.a.name() << " = " << "{ " << set.a.get()[0] << ", " << set.a.get()[1] << ", " << set.a.get()[2] << " }" << std::endl;
        std::cout << set.b.name() << " = " << set.b.get() << std::endl;
        std::cout << set.c.name() << " = " << set.c.get() << std::endl;
        std::cout << set.d.name() << " = " << set.d.get() << std::endl;
        std::cout << set.e.name() << " = " << set.e.get() << std::endl;
        std::cout << set.f.name() << " = " << set.f.get() << std::endl;
        std::cout << set.i.name() << " = " << set.i.get() << std::endl;
        std::cout << set.j.name() << " = " << set.j.get() << std::endl;
        std::cout << set.s.name() << ".x = " << set.s.get().x << std::endl;
        std::cout << set.s.name() << ".y = " << set.s.get().y << std::endl;
        std::cout << set.s.name() << ".z = " << set.s.get().z << std::endl;

        std::cout << "Accessing mirrors: " << std::endl;
        std::cout << set.a.name() << " = " << set.a << std::endl;
        std::cout << set.a.name() << " = " << "{ " << set.a[0] << ", " << set.a[1] << ", " << set.a[2] << " }" << std::endl;
        std::cout << set.b.name() << " = " << set.b << std::endl;
        std::cout << set.c.name() << " = " << set.c << std::endl;
        std::cout << set.d.name() << " = " << set.d << std::endl;
        std::cout << set.e.name() << " = " << set.e << std::endl;
        std::cout << set.f.name() << " = " << set.f << std::endl;
        std::cout << set.i.name() << " = " << set.i << std::endl;
        std::cout << set.j.name() << " = " << set.j << std::endl;
        std::cout << set.s.name() << "." << set.s.x.name() << " = " << set.s.x << std::endl;
        std::cout << set.s.name() << "." << set.s.y.name() << " = " << set.s.y << std::endl;
        std::cout << set.s.name() << "." << set.s.z.name() << " = " << set.s.z << std::endl;

        std::cout << "Print the whole struct: \n" << set;

        std::stringstream buffer;
        buffer << set;
        set_defaults(&settings);
        std::cout << "After set_defaults: \n" << set;

        while (!buffer.eof())
            buffer >> set;
        std::cout << "After parsing: \n" << set;

        // save into other settings:
        other_settings_t other;
        set.save_into(&other);

        set_defaults(&settings);
        std::cout << "After set_defaults: \n" << set;

        set.load_from(&other);
        std::cout << "After loading: \n" << set;

        list_fields<base_field>(set);
        list_fields<array_mirror>(set);
        list_fields<int_mirror>(set);
        list_fields<float_mirror>(set);
        list_fields<enum_mirror>(set);
        list_fields<struct_mirror>(set);
    }
    catch (const std::exception& s) {
        std::cerr << "Exception occurred: " << s.what() << std::endl;
    }

}