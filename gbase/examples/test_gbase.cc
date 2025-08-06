
#include "gbase.h"

constexpr const char* G1_LOGGER = "g1detector";


class g1 : public GBase<g1> {
public:

	int object1 = 2;

	explicit g1(const std::shared_ptr<GOptions>& gopt) : GBase(gopt, G1_LOGGER) {
		log->info(0, "hello derived class ");
	}

	void doSomething([[maybe_unused]] int a=0) {
		log->info(0, "doing something ");
		log->debug(NORMAL, FUNCTION_NAME, "debug message ");
		log->warning(FUNCTION_NAME, "warning message ");
	}

	~g1() = default;
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

	g1 obj(gopts);

	obj.doSomething();

	return EXIT_SUCCESS;

}
