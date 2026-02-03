/**
 * @file test_gbase.cc
 * @brief Example program demonstrating how to derive from \c GBase and use logging.
 *
 * This example shows two common usage patterns:
 * 1) A derived type that constructs its own logger via \c GOptions.
 * 2) A derived type that reuses an already constructed shared  GLogger .
 *
 * The focus is on illustrating:
 * - How derived classes access the protected \c log member from \ref GBase .
 * - How different logger naming/channel choices can be routed through options.
 * - How typical message categories (\c info, \c warning, \c debug) might be used.
 *
 * Notes:
 * - This file is intentionally an example/driver and does not provide authoritative API
 *   documentation for \ref GBase itself (see the header for API docs).
 * - The classes in this file are minimal and exist only to demonstrate construction
 *   patterns and log calls.
 */

#include "gbase.h"

constexpr const char* G1_LOGGER = "hello1";
constexpr const char* G2_LOGGER = "hello2";

// derived with its own logger
class g1 : public GBase<g1>
{
public:
	int object1 = 2;

	explicit g1(const std::shared_ptr<GOptions>& gopt) : GBase(gopt, G1_LOGGER) {
		log->info(0, "hello derived class ", SFUNCTION_NAME);
	}

	void doSomething([[maybe_unused]] int a = 0) {
		log->info(0, "doing something ");
		log->debug(NORMAL, FUNCTION_NAME, "debug message ");
		log->warning(FUNCTION_NAME, "warning message ");
	}

	~g1() = default;
};

// derived with shared logger
class g2 : public GBase<g2>
{
public:
	int object1 = 2;

	explicit g2(const std::shared_ptr<GLogger>& log) : GBase(log) {
		log->info(0, "hello derived class ", SFUNCTION_NAME);
	}

	void doSomething([[maybe_unused]] int a = 0) {
		log->info(0, "doing something ");
		log->debug(NORMAL, FUNCTION_NAME, "debug message ");
		log->warning(FUNCTION_NAME, "warning message ");
	}

	~g2() = default;
};


// returns this example options
GOptions defineOptions() {
	GOptions goptions(G1_LOGGER);
	goptions += GOptions(G2_LOGGER);

	// command line switch
	goptions.defineSwitch("light", "a switch, this is just an example.");

	return goptions;
}

int main(int argc, char* argv[]) {
	auto gopts = std::make_shared<GOptions>(argc, argv, defineOptions());
	auto log   = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, G2_LOGGER);

	g1 obj1(gopts);
	g2 obj2(log);


	obj1.doSomething();
	obj2.doSomething();

	return EXIT_SUCCESS;
}
