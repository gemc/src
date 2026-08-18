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
		GVariable("optional_string", std::nullopt, "optional string used by this test"),
		"An unset value must be returned as std::nullopt.");
	options.defineOption(
		GVariable("optional_int", std::nullopt, "optional integer used by this test"),
		"An unset value must be returned as std::nullopt.");
	return options;
}

} // namespace

int main(int argc, char* argv[]) {
	std::optional<std::string> expected;
	std::optional<int> expectedInt;
	const std::string option_prefix = "-optional_string=";
	const std::string int_option_prefix = "-optional_int=";
	for (int index = 1; index < argc; ++index) {
		const std::string argument = argv[index];
		if (argument.rfind(option_prefix, 0) == 0) {
			expected = argument.substr(option_prefix.size());
		}
		if (argument.rfind(int_option_prefix, 0) == 0) {
			expectedInt = std::stoi(argument.substr(int_option_prefix.size()));
		}
	}

	GOptions options(argc, argv, defineOptions());
	const auto value = options.getOptionalScalarString("optional_string");
	const auto intValue = options.getOptionalScalarInt("optional_int");

	if (expected) {
		if (!value || *value != *expected) return 1;
	}
	else if (value) return 1;
	return intValue == expectedInt ? 0 : 1;
}
