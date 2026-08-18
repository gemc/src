/**
 * \file optional_scalar.cc
 * \brief Verifies optional scalar-string access for absent and configured values.
 */

#include "goptions.h"

#include <optional>
#include <string>

namespace {

GOptions defineOptions() {
	GOptions options;
	options.defineOption(
		GVariable("optional_string", UNINITIALIZEDSTRINGQUANTITY, "optional string used by this test"),
		"An unset value must be returned as std::nullopt.");
	return options;
}

} // namespace

int main(int argc, char* argv[]) {
	std::optional<std::string> expected;
	const std::string option_prefix = "-optional_string=";
	for (int index = 1; index < argc; ++index) {
		const std::string argument = argv[index];
		if (argument.rfind(option_prefix, 0) == 0) {
			expected = argument.substr(option_prefix.size());
		}
	}

	GOptions options(argc, argv, defineOptions());
	const auto value = options.getOptionalScalarString("optional_string");

	if (expected) {
		return value && *value == *expected ? 0 : 1;
	}
	return value ? 1 : 0;
}
