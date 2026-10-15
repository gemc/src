#include <gemc/glogging/glogger.h>

#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void require(bool condition, const std::string& message) {
	if (!condition) throw std::runtime_error(message);
}

struct Parameter {
	std::string text;
	int* conversions;
};

std::ostream& operator<<(std::ostream& stream, const Parameter& parameter) {
	++*parameter.conversions;
	return stream << parameter.text;
}

class Capture {
public:
	std::ostringstream output;
	std::ostringstream errors;

	Capture() : output_buffer(G4cout.rdbuf(output.rdbuf())), error_buffer(G4cerr.rdbuf(errors.rdbuf())) {}
	~Capture() {
		G4cout.rdbuf(output_buffer);
		G4cerr.rdbuf(error_buffer);
	}

	void clear() {
		output.str("");
		errors.str("");
	}

private:
	std::streambuf* output_buffer;
	std::streambuf* error_buffer;
};

std::shared_ptr<GOptions> options(int verbosity, int debug) {
	auto result = std::make_shared<GOptions>();
	result->defineOption("verbosity", "", {{"templates", verbosity, ""}}, "");
	result->defineOption("debug", "", {{"templates", debug, ""}}, "");
	return result;
}

void test_templates() {
	Capture capture;
	GLogger logger;
	int counter = 0;
	auto expect = [&](const std::string& expected) {
		require(capture.output.str() == " [  - " + std::to_string(++counter) + " ] INFO L0: " + expected + "\n",
		        "Unexpected formatted output: " + capture.output.str());
		require(capture.errors.str().empty(), "Info message wrote to stderr");
		capture.clear();
	};

	logger.info(0, "Using %1 threads out of %2 available cores.", 4, 8);
	expect("Using 4 threads out of 8 available cores.");
	logger.info("%2 then %1 then %2", "first", "second");
	expect("second then first then second");
	logger.info("%1 %10 %2 %3 %4 %5 %6 %7 %8 %9", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
	expect("1 10 2 3 4 5 6 7 8 9");
	std::string all_parameters;
	std::string all_values;
	for (int index = 1; index <= 99; ++index) {
		all_parameters += "%" + std::to_string(index) + " ";
		all_values += std::to_string(index) + " ";
	}
	[&]<std::size_t... Indices>(std::index_sequence<Indices...>) {
		logger.info(all_parameters, (Indices + 1)...);
	}(std::make_index_sequence<99>{});
	expect(all_values);
	logger.info("%1|%2", "contains %2 and %%", "contains %1");
	expect("contains %2 and %%|contains %1");
	logger.info("100%%; %%1; %word; trailing %");
	expect("100%; %1; %word; trailing %");
	logger.info("%1%% complete", 50);
	expect("50% complete");
	logger.info("%1 / %2 / %3 / %4", std::string("value"), 1.25, true, 'x');
	expect("value / 1.25 / 1 / x");
	logger.info("日本語: %1; français: %2", "イベント", "événement");
	expect("日本語: イベント; français: événement");
	logger.info("");
	expect("");

	int conversions = 0;
	logger.info("%1 repeated %1", Parameter{"custom", &conversions});
	expect("custom repeated custom");
	require(conversions == 1, "Repeated parameter was converted more than once");

	auto invalid = [&](auto&& action) {
		bool rejected = false;
		try { action(); }
		catch (const std::invalid_argument&) { rejected = true; }
		require(rejected, "Malformed template was accepted");
		require(capture.output.str().empty() && capture.errors.str().empty(),
		        "Malformed template emitted partial output");
	};
	invalid([&] { logger.info("%1"); });
	invalid([&] { logger.info("%2", "one"); });
	invalid([&] { logger.info("%0", "one"); });
	invalid([&] { logger.info("%01", "one"); });
	invalid([&] { logger.info("%100", "one"); });
	invalid([&] { logger.info("%9999999999999999999999999999", "one"); });
	invalid([&] { logger.info("no placeholder", "unused"); });
	invalid([&] { logger.info("%1", "one", "unused"); });
	invalid([&] { logger.info(0, "Fragment ", 12); });

	logger.info(0, "Canonical message %1", 12);
	expect("Canonical message 12");
}

void test_filtering_and_categories() {
	Capture capture;
	for (int verbosity = 0; verbosity <= 2; ++verbosity) {
		GLogger logger(options(verbosity, 0), "test", "templates");
		for (int level = 0; level <= 2; ++level) {
			capture.clear();
			int conversions = 0;
			logger.info(level, "%1", Parameter{"value", &conversions});
			const bool enabled = level <= verbosity;
			require(conversions == (enabled ? 1 : 0), "Filtered info formatted a parameter");
			require(capture.output.str().empty() == !enabled, "Incorrect info verbosity filtering");
			if (enabled) {
				require(capture.output.str().find("INFO L" + std::to_string(level) + ": value\n") !=
				        std::string::npos, "Info template lost its level");
			}
			else { logger.info(level, "%0"); }
		}
	}

	GLogger quiet(options(0, 0), "test", "templates");
	int conversions = 0;
	capture.clear();
	quiet.debug(NORMAL, "%1", Parameter{"value", &conversions});
	quiet.debug(NORMAL, "%0");
	require(conversions == 0 && capture.output.str().empty(), "Disabled debug formatted a message");

	GLogger verbose(options(2, 1), "test", "templates");
	for (auto type : {NORMAL, CONSTRUCTOR, DESTRUCTOR}) {
		capture.clear();
		verbose.debug(type, "Debug %1", 7);
		require(capture.output.str().find("DEBUG: ") != std::string::npos, "Missing debug header");
		require(capture.output.str().find("Debug 7") != std::string::npos, "Missing debug message");
		if (type != NORMAL) {
			const auto marker = type == CONSTRUCTOR ? guts::CONSTRUCTORLOG : guts::DESTRUCTORLOG;
			require(capture.output.str().find(marker) != std::string::npos, "Missing lifecycle marker");
		}
	}
	capture.clear();
	quiet.warning("Warning %1", 8);
	require(capture.output.str().find(guts::GWARNING) != std::string::npos, "Missing warning header");
	require(capture.output.str().find("Warning 8") != std::string::npos, "Warning was filtered");
	capture.clear();
	quiet.critical("Critical %1", 9);
	require(capture.output.str().find("Critical 9") != std::string::npos, "Critical was filtered");
	require(capture.errors.str().empty(), "Nonfatal category wrote to stderr");
}

void test_aliases() {
	Capture capture;
	GLogger canonical(options(2, 1), "test", "templates");
	GLogger alias(options(2, 1), "test", "templates");
	const std::string value = "literal %2 and %%";
	auto same = [&](auto&& normal, auto&& transitional) {
		capture.clear();
		normal();
		const auto expected = capture.output.str();
		capture.clear();
		transitional();
		require(capture.output.str() == expected, "Transitional alias changed formatted output");
		require(capture.errors.str().empty(), "Nonfatal alias wrote to stderr");
	};
	same([&] { canonical.info(0, "%1", value); }, [&] { alias.infoTemplate(0, "%1", value); });
	same([&] { canonical.info("%1", value); }, [&] { alias.infoTemplate("%1", value); });
	for (auto type : {NORMAL, CONSTRUCTOR, DESTRUCTOR}) {
		same([&] { canonical.debug(type, "%1", value); },
		     [&] { alias.debugTemplate(type, "%1", value); });
	}
	same([&] { canonical.warning("%1", value); }, [&] { alias.warningTemplate("%1", value); });
	same([&] { canonical.critical("%1", value); }, [&] { alias.criticalTemplate("%1", value); });
}

} // namespace

int main(int argc, char* argv[]) {
	const std::string mode = argc > 1 ? argv[1] : "";
	GLogger logger;
	if (mode == "error") { logger.error(42, "File missing%%2.txt not found."); }
	if (mode == "error-template") { logger.errorTemplate(42, "File %1 not found.", "missing%2.txt"); }
	if (mode == "invalid-level") { logger.info(3, "unused"); }
	if (mode == "invalid-level-template") { logger.infoTemplate(3, "%0"); }
	try {
		test_templates();
		test_filtering_and_categories();
		test_aliases();
	}
	catch (const std::exception& error) {
		std::cerr << error.what() << '\n';
		return 1;
	}
	return 0;
}
