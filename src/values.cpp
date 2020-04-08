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

base_field& base_struct::at(const char *name)
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

void base_struct::print(std::ostream& str) const
{
	for (auto& field : *this) {
		str << field.name << " = " << field.value << std::endl;
	}
}

INTROSPECT_NS_CLOSE;