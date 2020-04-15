#pragma once

#define INTROSPECT_NS_OPEN namespace introspect {
#define INTROSPECT_NS_CLOSE }

#include <stdint.h>

INTROSPECT_NS_OPEN;

template<typename T, typename Enable = void>
struct mirror;

struct base_mirror;
class variant;
struct enum_mirror;
struct array_mirror;

struct visitor;
struct const_visitor;

struct base_field;
struct base_fields;
struct struct_mirror;

struct enum_value;

INTROSPECT_NS_CLOSE;
