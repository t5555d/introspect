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


struct other_settings_t
{
    int32_t A;
    int32_t B;
    int32_t C;
    int32_t D[3];
    int32_t EX;
    int32_t EY;
    int32_t EZ;
    double  F;
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



struct settings_t;

STRUCT_FIELDS(settings_t)
{
    STRUCT_FIELD2(a, int, with_default(0));
    STRUCT_FIELD2(b, int64_t, with_default(0LL));
    STRUCT_FIELD2(c, enum_t, with_default(VALUE0));
    STRUCT_FIELD2(d, int[3]); /*with_default(-1)*/
    STRUCT_FIELD2(e, point_t);
    STRUCT_FIELD2(f, double, with_default(0.0));
};

struct settings_t : struct_fields<settings_t, raw_fields> {};

void set_defaults(settings_t* set)
{
    memset(set, DEFAULT_VALUE, sizeof(settings_t));
}

// generic introspective settings

using settings_c = mirror<settings_t, simple_fields>;

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
    settings.f = 50.55;

    try {
        settings_c set(settings);

        std::cout << "Accessing raw values: " << std::endl;
        std::cout << set.a.name() << " = " << set.a.get() << std::endl;
        std::cout << set.b.name() << " = " << set.b.get() << std::endl;
        std::cout << set.c.name() << " = " << set.c.get() << std::endl;
        std::cout << set.d.name() << " = " << set.d.get() << std::endl;
        std::cout << set.d.name() << " = "
            << "{ " << set.d.get()[0]
            << ", " << set.d.get()[1]
            << ", " << set.d.get()[2]
            << " }" << std::endl;
        std::cout << set.e.name() << ".x = " << set.e.get().x << std::endl;
        std::cout << set.e.name() << ".y = " << set.e.get().y << std::endl;
        std::cout << set.e.name() << ".z = " << set.e.get().z << std::endl;
        std::cout << set.f.name() << " = " << set.f.get() << std::endl;

        std::cout << "Accessing mirrors: " << std::endl;
        std::cout << set.a.name() << " = " << set.a << std::endl;
        std::cout << set.b.name() << " = " << set.b << std::endl;
        std::cout << set.c.name() << " = " << set.c << std::endl;
        std::cout << set.d.name() << " = " << set.d << std::endl;
        std::cout << set.d.name() << " = "
            << "{ " << set.d[0]
            << ", " << set.d[1]
            << ", " << set.d[2]
            << " }" << std::endl;

        std::cout << set.e.name() << "." << set.e.x.name() << " = " << set.e.x << std::endl;
        std::cout << set.e.name() << "." << set.e.y.name() << " = " << set.e.y << std::endl;
        std::cout << set.e.name() << "." << set.e.z.name() << " = " << set.e.z << std::endl;
        std::cout << set.f.name() << " = " << set.f << std::endl;

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