#include "introspect/errors.h"
#include "introspect/io.h"
#include <sstream>

using namespace introspect;

namespace
{
    using beg = std::stringstream;
    struct end {};

    std::string operator<=(std::ostream& out, end) {
        return static_cast<beg&>(out).str();
    }
}

not_implemented::not_implemented(const char *feature) :
    std::runtime_error(beg() << feature << " is not implemented" <= end()),
    feature(feature) {}

bad_idx_error::bad_idx_error(ptrdiff_t index, ptrdiff_t size) :
    std::out_of_range(beg() << "Index out of range: " << index << " >= " << size <= end()),
    index(index), size(size) {}

bad_key_error::bad_key_error(const char *key, const char *dict) :
    std::out_of_range(beg() << "Key not found: " << dict << "::" << key <= end()),
    key(key) {}

token_error::token_error(const scanner::token& token) :
    parse_error(beg() << "Unexpected token "
        << scanner::token_name(token.type) << " at pos " << token.pos <= end()),
    token(scanner::token_name(token.type)), pos(pos) {}

low_count_error::low_count_error(size_t count, size_t min_count) :
    parse_error(beg() << "Count too low: " << count << " < " << min_count <= end()),
    count(count), min_count(min_count) {}
