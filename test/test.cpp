#include <iostream>
#include <sstream>
#include <utility>
#include <stdint.h>
#include <gtest/gtest.h>
#include "introspect/fields.h"
#include "introspect/attrib.h"
#include "introspect/io.h"

using namespace introspect;

// simple settings

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
    int32_t a[3];
    int32_t b;
    int32_t c;
    double d;
    int32_t e;
    double f;
    int32_t i;
    int64_t j;
    point_t s;
    int32_t x, y, z;
};

ENUM_OPTIONS(enum_t)
{
    ENUM_OPTION(VALUE0);
    ENUM_OPTION(VALUE1);
};

STRUCT_FIELDS(point_t)
{
    STRUCT_FIELD(x, with_name("X"), maps_to(&other_settings_t::x));
    STRUCT_FIELD(y, with_name("Y"), maps_to(&other_settings_t::y));
    STRUCT_FIELD(z, with_name("Z"), maps_to(&other_settings_t::z));
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
    STRUCT_FIELD2(p, point_t,   maps_to(&other_settings_t::s));
    STRUCT_FIELD2(s, point_t,   maps_to<other_settings_t>());
};

struct settings_t : struct_fields<settings_t, raw_fields> {};

// generic introspective settings

using settings_c = mirror<settings_t, simple_fields>;

void set_default(settings_t& set)
{
    memset(&set, 0, sizeof(settings_t));
}

void set_example(settings_t& settings)
{
    set_default(settings);
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
    settings.p.x = 10;
    settings.p.y = 11;
    settings.p.z = 12;
    settings.s.x = 13;
    settings.s.y = 14;
    settings.s.z = 15;
}

bool operator==(const settings_t& a, const settings_t& b)
{
    return 0 == memcmp(&a, &b, sizeof(settings_t));
}

TEST(IO, CompareOutput)
{
    settings_t settings;
    set_example(settings);

    settings_c set(settings);

    std::ostringstream out0, out1, out2;
    out0 << set.a.name() << " = " << "{ " << set.a.get()[0] << ", " << set.a.get()[1] << ", " << set.a.get()[2] << " }" << std::endl;
    out0 << set.b.name() << " = " << set.b.get() << std::endl;
    out0 << set.c.name() << " = " << +set.c.get() << std::endl;
    out0 << set.d.name() << " = " << set.d.get() << std::endl;
    out0 << set.e.name() << " = " << (set.e.get() ? "VALUE1" : "VALUE0") << std::endl;
    out0 << set.f.name() << " = " << set.f.get() << std::endl;
    out0 << set.i.name() << " = " << set.i.get() << std::endl;
    out0 << set.j.name() << " = " << set.j.get() << std::endl;
    out0 << set.p.name() << ".X = " << set.p.get().x << std::endl;
    out0 << set.p.name() << ".Y = " << set.p.get().y << std::endl;
    out0 << set.p.name() << ".Z = " << set.p.get().z << std::endl;
    out0 << set.s.name() << ".X = " << set.s.get().x << std::endl;
    out0 << set.s.name() << ".Y = " << set.s.get().y << std::endl;
    out0 << set.s.name() << ".Z = " << set.s.get().z << std::endl;

    out1 << set.a.name() << " = " << "{ " << set.a[0] << ", " << set.a[1] << ", " << set.a[2] << " }" << std::endl;
    out1 << set.b.name() << " = " << set.b << std::endl;
    out1 << set.c.name() << " = " << set.c << std::endl;
    out1 << set.d.name() << " = " << set.d << std::endl;
    out1 << set.e.name() << " = " << set.e << std::endl;
    out1 << set.f.name() << " = " << set.f << std::endl;
    out1 << set.i.name() << " = " << set.i << std::endl;
    out1 << set.j.name() << " = " << set.j << std::endl;
    out1 << set.p.name() << "." << set.p.x.name() << " = " << set.p.x << std::endl;
    out1 << set.p.name() << "." << set.p.y.name() << " = " << set.p.y << std::endl;
    out1 << set.p.name() << "." << set.p.z.name() << " = " << set.p.z << std::endl;
    out1 << set.s.name() << "." << set.s.x.name() << " = " << set.s.x << std::endl;
    out1 << set.s.name() << "." << set.s.y.name() << " = " << set.s.y << std::endl;
    out1 << set.s.name() << "." << set.s.z.name() << " = " << set.s.z << std::endl;

    out2 << set;

    EXPECT_EQ(out0.str(), out2.str());
    EXPECT_EQ(out1.str(), out2.str());
}

TEST(IO, SaveLoadCompare)
{
    settings_t settings1, settings2;

    set_example(settings1);
    set_default(settings2);

    settings_c set(settings1);

    // save
    std::stringstream buffer;
    buffer << set;

    // load
    set.addr(&settings2);
    while (!buffer.eof())
        buffer >> set;

    // compare:
    EXPECT_EQ(settings1, settings2);
}

TEST(Mapping, SaveLoadCompare)
{
    settings_t settings1, settings2;

    set_example(settings1);
    set_default(settings2);

    settings_c set(settings1);

    // save
    other_settings_t buffer;
    set.save_into(&buffer);

    // load
    set.addr(&settings2);
    set.load_from(&buffer);

    // compare:
    EXPECT_EQ(settings1, settings2);
}

template<typename Field, typename Fields>
void test_fields(const Fields& m, const std::set<const base_field *> expected)
{
    std::set<const base_field*> actual;
    for (const Field& field : m.fields<Field>())
        actual.insert(dynamic_cast<const base_field *>(&field));

    EXPECT_EQ(actual, expected);
}

TEST(Fields, Enumerate)
{
    settings_c set;

    test_fields<base_field>   (set, { &set.a, &set.b, &set.c, &set.d, &set.e, &set.f, &set.i, &set.j, &set.p, &set.s });
    test_fields<array_mirror> (set, { &set.a });
    test_fields<int_mirror>   (set, { &set.b, &set.c, &set.e, &set.i, &set.j });
    test_fields<float_mirror> (set, { &set.d, &set.f });
    test_fields<enum_mirror>  (set, { &set.e });
    test_fields<struct_mirror>(set, { &set.p, &set.s });
}
