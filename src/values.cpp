#include "introspect/values.h"
#include "introspect/fields.h"
#include <sstream>

INTROSPECT_NS_OPEN;

variant base_array::at(size_t i) {
	size_t len = count();
	if (i >= len) {
		std::ostringstream err;
		err << "base_array::at: illegal index " << i << " >= " << len;
		throw std::out_of_range(err.str());
	}
	return operator[](i);
}

void base_array::parse(std::istream& str)
{
	throw std::runtime_error("not implemented");
}

void base_array::print(std::ostream& str) const
{
	auto& self = *this;
	str << "{ " << self[0];
	for (size_t i = 1, n = count(); i < n; i++)
		str << ", " << self[i];
	str << " }";
}

void base_enum::parse(std::istream& str)
{
    if (isalpha(str.peek())) {
		std::string id;
		str >> id;
		for (auto& pair : values()) {
			if (id == pair.name) {
                return int_value(pair.value);
			}
		}
        throw parse_error(std::string("unknown enum value: ") + id);
	}
	else {
        int64_t value;
        str >> value;
        return int_value(value);
    }
}

void base_enum::print(std::ostream& str) const
{
    auto value = int_value();
	for (auto& pair : values()) {
		if (pair.value == value) {
			str << pair.name;
			return;
		}
	}
	str << value;
}



base_field& base_fields::at(const char *name)
{
	for (auto& field : *this) {
		if (0 == strcmp(field.name, name))
			return field;
	}
	throw std::out_of_range("base_struct: failed to find field");
}

void base_struct::parse(std::istream& str)
{
	throw std::runtime_error("not implemented");
}

void base_struct::print(std::ostream& str, const char *prefix) const
{
	for (auto& field : fields()) {
        auto pstruct = dynamic_cast<base_struct *>(&field.value);
        if (pstruct) {
            char full_name[256];
            snprintf(full_name, sizeof(full_name), "%s%s.", prefix, field.name);
            pstruct->print(str, full_name);
        }
        else {
            str << prefix << field.name << " = " << field.value << std::endl;
        }
	}
}

INTROSPECT_NS_CLOSE;