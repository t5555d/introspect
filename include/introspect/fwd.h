#pragma once

#define INTROSPECT_NS_OPEN namespace introspect {
#define INTROSPECT_NS_CLOSE }

#include <stdint.h>

INTROSPECT_NS_OPEN;

template<typename T, typename Enable = void>
struct mirror;

struct base_mirror;
class variant;
struct int_mirror;
struct enum_mirror;
struct float_mirror;
struct array_mirror;
struct struct_mirror;

struct visitor;
struct const_visitor;

struct base_field;
struct base_fields;

struct enum_value;

INTROSPECT_NS_CLOSE;
