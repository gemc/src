
#include "gbase.h"

constexpr const char* G1_LOGGER = "gdetector";
constexpr const char* G2_LOGGER = "gfield";

// derived with its own logger
class g1 : public GBase<g1> {
public:

	int object1 = 2;

	explicit g1(const std::shared_ptr<GOptions>& gopt) : GBase(gopt, G1_LOGGER) {
		log->info(0, "hello derived class ", SFUNCTION_NAME);
	}

	void doSomething([[maybe_unused]] int a=0) {
		log->info(0, "doing something ");
		log->debug(NORMAL, FUNCTION_NAME, "debug message ");
		log->warning(FUNCTION_NAME, "warning message ");
	}

	~g1() = default;
};

// derived with shared logger
class g2 : public GBase<g2> {
public:

	int object1 = 2;

	explicit g2(const std::shared_ptr<GLogger>& log) : GBase(log) {
		log->info(0, "hello derived class ", SFUNCTION_NAME);
	}

	void doSomething([[maybe_unused]] int a=0) {
		log->info(0, "doing something ");
		log->debug(NORMAL, FUNCTION_NAME, "debug message ");
		log->warning(FUNCTION_NAME, "warning message ");
	}

	~g2() = default;
};


// returns this example options
GOptions defineOptions() {

	GOptions goptions(G1_LOGGER);

	// command line switch
	goptions.defineSwitch("light", "a switch, this is just an example.");

	return goptions;
}

int main(int argc, char *argv[]) {

	auto gopts = std::make_shared<GOptions>(argc, argv, defineOptions());
	auto log   = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, G2_LOGGER);

	g1 obj1(gopts);
	g2 obj2(log);


	obj1.doSomething();
	obj2.doSomething();

	return EXIT_SUCCESS;

}
