#pragma once

#include "fwd.h"
#include <stdexcept>

INTROSPECT_NS_OPEN;

struct not_implemented : std::runtime_error
{
    not_implemented(const char *feature);
    const char *feature;
};

struct bad_idx_error : std::out_of_range
{
    bad_idx_error(ptrdiff_t index, ptrdiff_t size);
    const ptrdiff_t index;
    const ptrdiff_t size;
};

struct bad_key_error : std::out_of_range
{
    bad_key_error(const char *key, const char *dict);
    const std::string key;
};

struct parse_error : std::runtime_error
{
    parse_error(const std::string& message) : 
        std::runtime_error(message) {}
};

INTROSPECT_NS_CLOSE;
