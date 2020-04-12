#pragma once

#define INTROSPECT_NS_OPEN namespace introspect {
#define INTROSPECT_NS_CLOSE }

#include <stdint.h>

INTROSPECT_NS_OPEN;

template<typename T, typename Enable = void>
struct mirror;

struct base_mirror;
class variant;
struct base_enum;
struct base_array;

struct base_field;
struct base_fields;
struct base_struct;

struct enum_value;

INTROSPECT_NS_CLOSE;
