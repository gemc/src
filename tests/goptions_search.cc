// Distinct terms isolate the searchable fields from one another.
#include "goptions.h"

int main(int argc, char* argv[]) {
	GOptions definitions;
	definitions.defineSwitch("fixture_switch", "toggle annotation");
	definitions.defineOption(
		GVariable("fixture_scalar", std::string("scalar_initial"), "scalar synopsis"),
		"scalar extended guidance");
	definitions.defineOption("fixture_schema", "schema synopsis", {
		{"schema_key", std::string("schema_initial"), "field annotation"},
		{"optional_key", std::nullopt, "optional annotation"},
		{"count_key", 1739, "integer annotation"},
		{"enabled_key", true, "boolean annotation"}
	}, "schema extended guidance");
	definitions.defineOption("fixture_cumulative", "cumulative synopsis", {
		{"required_key", goptions::REQUIRED, "mandatory annotation"},
		{"preset_key", std::string("cumulative_initial"), "preset annotation"}
	}, "cumulative extended guidance");
	GOptions options(argc, argv, definitions);
	return EXIT_SUCCESS;
}
