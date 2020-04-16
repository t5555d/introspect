#pragma once

#include "values.h"
#include <iostream>
#include <list>

INTROSPECT_NS_OPEN;

struct print_visitor : const_visitor
{
    explicit print_visitor(std::ostream& str) :
        out(str) {}

    void visit(const int_mirror& value) override;
    void visit(const float_mirror& value) override;
    void visit(const enum_mirror& value) override;
    void visit(const array_mirror& value) override;
    void visit(const struct_mirror& value) override;

private:

    std::list<const char *> context;
    std::ostream& out;

    const char *end() const {
        return context.empty() ? "" : "\n";
    }
};

struct parse_visitor : visitor
{
    explicit parse_visitor(std::istream& str) :
        input(str) {}

    void visit(int_mirror& value) override;
    void visit(enum_mirror& value) override;

private:
    std::istream& input;
};

inline std::istream& operator >> (std::istream& str, base_mirror& value)
{
    value.visit(parse_visitor(str));
    return str;
}

inline std::ostream& operator << (std::ostream& str, const base_mirror& value)
{
    value.visit(print_visitor(str));
    return str;
}

INTROSPECT_NS_CLOSE;