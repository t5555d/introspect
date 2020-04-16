#include "introspect/io.h"
#include "introspect/fields.h"
#include <string>

INTROSPECT_NS_OPEN;

//
// printer
//

using context_t = std::list<const char *>;

static std::ostream& operator<<(std::ostream& out, const context_t& context)
{
    auto i = context.begin();
    if (i != context.end()) {
        out << *i;
        while (++i != context.end())
            out << "." << *i;
        out << " = ";
    }
    return out;
};

void print_visitor::visit(const int_mirror& value)
{
    out << context << value.int_value() << end();
}

void print_visitor::visit(const float_mirror& value)
{
    out << context << value.float_value() << end();
}

void print_visitor::visit(const enum_mirror& value)
{
    out << context;
    auto int_value = value.int_value();
    for (auto& pair : value.values()) {
        if (pair.value == int_value) {
            out << pair.name << end();
            return;
        }
    }
    out << int_value << end();
}

void print_visitor::visit(const array_mirror& value)
{
    out << context << "{ " << value[0];
    for (size_t i = 1, n = value.count(); i < n; i++)
        out << ", " << value[i];
    out << " }" << end();
}

void print_visitor::visit(const struct_mirror& value)
{
    for (auto& field : value.fields()) {
        context.push_back(field.name);
        field.value.visit(*this);
        context.pop_back();
    }
}

//
// parser
//

void parse_visitor::visit(int_mirror& value)
{
    int64_t x;
    input >> x;
    value.int_value(x);
}

void parse_visitor::visit(enum_mirror& value)
{
    if (isalpha(input.peek())) {
        std::string id;
        input >> id;
        for (auto& pair : value.values()) {
            if (id == pair.name)
                return value.int_value(pair.value);
        }
        throw std::runtime_error(std::string("unknown enum value: ") + id);
    }

    return visit(static_cast<int_mirror&>(value));
}

INTROSPECT_NS_CLOSE;
