#include "introspect/io.h"
#include "introspect/fields.h"
#include "introspect/errors.h"
#include <string>

INTROSPECT_NS_OPEN;

//
// printer
//

std::ostream& operator<<(std::ostream& out, const context_t& context)
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
        context.push(field.name());
        field.value.visit(*this);
        context.pop();
    }
}

//
// scanner
//

size_t scanner::read_while(char *buf, size_t buf_size, char_pred cond)
{
    for (size_t count = 0, max_count = buf_size - 1; count < max_count; count++) {
        auto c = input.peek();
        if (cond(c))
            buf[count] = input.get();
        else {
            buf[count] = 0;
            return count;
        }
    }
    return buf_size;
}

void scanner::skip_while(char_pred cond)
{
    while (true) {
        auto c = input.peek();
        if (!cond(c))
            break;
        input.get();
    }
}

scanner::token scanner::read()
{
    skip_while(myspace);

    position_t pos = input.tellg();
    auto c = input.get();

    if (c == '\n' || c == std::istream::traits_type::eof())
        return{ pos, scanner::EOL };

    if (c == '.' || c == '=' || c == '{' || c == '}' || c == ',')
        return{ pos, c };

    if (isalpha(c)) {
        input.unget();
        read_while(token_text, MAX_TOKEN_LENGTH, isalnum);
        return{ pos, token_text };
    }

    if (isdigit(c) || c == '-' || c == '+') {
        char *end = token_text;
        *end++ = c;

        if (c == '-' || c == '+') {
            c = input.get();
            if (!isdigit(c))
                throw token_error({ scanner::position_t(input.tellg()), c });
            *end++ = c;
        }

        if (c == '0' && tolower(input.peek()) == 'x') {
            *end++ = input.get();
            end += read_while(end, 16, isxdigit);
            int64_t value = strtoll(token_text, nullptr, 16);
            return{ pos, value };
        }

        end += read_while(end, 32, isdigit);

        if (input.peek() == '.') {
            *end++ = input.get();
            end += read_while(end, 32, isdigit);
            double value = strtod(token_text, nullptr);
            return{ pos, value };
        }

        int64_t value = strtoll(token_text, nullptr, 10);
        return{ pos, value };
    }

    throw token_error({ pos, c });
}

scanner::token scanner::expect_impl(const int *expected_type, const int *end)
{
    auto& t = peek();
    for (; expected_type != end; expected_type++) {
        if (t.type == *expected_type)
            return get();
    }
    throw token_error(t);
}

std::string scanner::token_name(int type) {
    if (type < EOL) {
        char buf[] = { '\'', char(type), '\'', '\0' };
        return buf;
    }
    static const char *names[] = {
        "end-of-line",
        "identifier",
        "integer",
        "float"
    };
    return names[type - EOL];
}

//
// parser
//

void parse_visitor::visit(int_mirror& value)
{
    auto token = input.expect(scanner::INT);
    value.int_value(token.int_value);
}

void parse_visitor::visit(float_mirror& value)
{
    auto token = input.expect(scanner::INT, scanner::FLOAT);
    value.float_value(token.type == scanner::INT ? 
        token.int_value : token.float_value);
}

void parse_visitor::visit(enum_mirror& value)
{
    auto token = input.expect(scanner::INT, scanner::NAME);
    if (token.type == scanner::NAME) {
        for (auto& var : value.values()) {
            if (0 == strcmp(var.name, token.name))
                return value.int_value(var.value);
        }
        throw bad_key_error(token.name, value.type());
    }
    value.int_value(token.int_value);
}

void parse_visitor::visit(array_mirror& value)
{
    int brace = input.peek().type == '{' ? '}' : 0;
    if (brace) // skip opening brace
        input.get();

    value[0].visit(*this);

    for (size_t i = 1, n = value.count(); i < n; i++) {
        auto delim = input.expect(',', brace ? brace : scanner::EOL);
        if (delim.type != ',') {
            input.unget(delim);
            break;
        }
        value[i].visit(*this);
    }

    if (brace) // expect closing brace
        input.expect(brace);
}

void parse_visitor::visit(struct_mirror& value)
{
    auto name = input.expect(scanner::NAME, scanner::EOL);
    if (name.type == scanner::EOL)
        return;
    auto& field = value.fields()[name.name];

    auto *nested_struct = dynamic_cast<struct_mirror *>(&field.value);
    input.expect(nested_struct ? '.' : '=');

    context.push(field.name());
    field.value.visit(*this);
    context.pop();

    if (context.empty())
        input.expect(scanner::EOL);
}

INTROSPECT_NS_CLOSE;
