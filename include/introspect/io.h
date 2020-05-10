#pragma once

#include "values.h"
#include <iostream>
#include <list>

INTROSPECT_NS_OPEN;

struct context_t
{
    using impl_t = std::list<const base_field *>;

    void push(const base_field& field) { names.push_back(&field); }
    void pop() { names.pop_back(); }

    impl_t::const_iterator begin() const { return names.begin(); }
    impl_t::const_iterator end() const { return names.end(); }
    bool empty() const { return names.empty(); }

private:
    impl_t names;
};

std::ostream& operator<<(std::ostream& out, const context_t& context);

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

    context_t context;
    std::ostream& out;

    const char *end() const {
        return context.empty() ? "" : "\n";
    }
};

struct scanner
{
    scanner(std::istream& str) :
        input(str), next_token(0, '\0') {}

    enum token_type_t {
        EOL = 256,
        NAME,
        INT,
        FLOAT,
    };

    static std::string token_name(int type);

    using position_t = uint64_t;

    struct token {
        position_t pos;
        int type;

        token(position_t pos, const char *value) :
            pos(pos), type(NAME), name(value) {}

        token(position_t pos, int64_t value) :
            pos(pos), type(INT), int_value(value) {}

        token(position_t pos, double value) :
            pos(pos), type(FLOAT), float_value(value) {}

        token(position_t pos, int value) :
            pos(pos), type(value) {}

        union {
            const char *name;
            int64_t int_value;
            double float_value;
        };
    };

    const token& peek() {
        if (!next_read) {
            next_token = read();
            next_read = true;
        }
        return next_token;
    }
    
    token get() {
        if (!next_read)
            return read();
        next_read = false;
        return next_token;
    }

    bool unget(token t) {
        if (next_read)
            return false;
        next_token = t;
        next_read = true;
        return true;
    }

    template<typename... TokenType>
    token expect(TokenType... token_types) {
        int expected_types[] = { token_types... };
        return expect_impl(std::begin(expected_types), std::end(expected_types));
    }

private:
    static constexpr size_t MAX_TOKEN_LENGTH = 256;

    std::istream& input;
    char token_text[MAX_TOKEN_LENGTH];

    token read();

    token next_token;
    bool next_read = false;

    using char_pred = int (*) (int c);
    static int sameline(int c) { return c != '\n'; }
    static int myspace(int c) { return isspace(c) && c != '\n'; }

    size_t read_while(char *buf, size_t buf_size, char_pred cond);
    void skip_while(char_pred cond);

    token expect_impl(const int *beg, const int *end);
};

struct parse_visitor : visitor
{
    explicit parse_visitor(std::istream& str) :
        input(str) {}

    void visit(int_mirror& value) override;
    void visit(enum_mirror& value) override;
    void visit(float_mirror& value) override;
    void visit(array_mirror& value) override;
    void visit(struct_mirror& value) override;

private:
    scanner input;
    context_t context;
};

struct parse_error : std::runtime_error
{
    parse_error(const std::string& message) :
        std::runtime_error(message) {}
};

struct token_error : parse_error
{
    token_error(const scanner::token& token);

    std::string token;
    uint64_t pos;
};

struct low_count_error : parse_error
{
    low_count_error(size_t count, size_t min_count);
    size_t count;
    size_t min_count;
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