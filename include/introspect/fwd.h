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
struct base_struct;

INTROSPECT_NS_CLOSE;
