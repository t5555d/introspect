#include "introspect/values.h"
#include "introspect/fields.h"
#include "introspect/io.h"
#include "introspect/errors.h"
#include <sstream>

INTROSPECT_NS_OPEN;

void visitor::visit(base_mirror& value) { throw not_implemented(__FUNCTION__); }
void visitor::visit(int_mirror& value) { visit(static_cast<base_mirror&>(value)); }
void visitor::visit(enum_mirror& value) { visit(static_cast<int_mirror&>(value)); }
void visitor::visit(float_mirror& value) { visit(static_cast<base_mirror&>(value)); }
void visitor::visit(array_mirror& value) { visit(static_cast<base_mirror&>(value)); }
void visitor::visit(struct_mirror& value) { visit(static_cast<base_mirror&>(value)); }

void const_visitor::visit(const base_mirror& value) { throw not_implemented(__FUNCTION__); }
void const_visitor::visit(const int_mirror& value) { visit(static_cast<const base_mirror&>(value)); }
void const_visitor::visit(const enum_mirror& value) { visit(static_cast<const int_mirror&>(value)); }
void const_visitor::visit(const float_mirror& value) { visit(static_cast<const base_mirror&>(value)); }
void const_visitor::visit(const array_mirror& value) { visit(static_cast<const base_mirror&>(value)); }
void const_visitor::visit(const struct_mirror& value) { visit(static_cast<const base_mirror&>(value)); }

variant::variant(variant&& var)
{
    if (var.is_inline()) {
        memcpy(buffer, var.buffer, VARIANT_BUFFER_SIZE);
        size_t offset = reinterpret_cast<uint8_t *>(var.value) - var.buffer;
        value = reinterpret_cast<base_mirror *>(buffer + offset);
    }
    else
        value = var.value;
    var.value = nullptr;
}

variant::~variant() {
    if (value == nullptr)
        ; // nothing to do
    else if (is_inline())
        value->~base_mirror();
    else
        delete value;
}

variant array_mirror::at(size_t i) {
    size_t len = count();
    if (i >= len)
        throw bad_idx_error(i, len);
    return operator[](i);
}

base_field& struct_mirror::at(const char *name)
{
    for (auto& field : fields()) {
        if (0 == strcmp(field.name(), name))
            return field;
    }
    throw bad_key_error(name, type());
}

INTROSPECT_NS_CLOSE;