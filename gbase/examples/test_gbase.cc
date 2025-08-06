
#include "gbase.h"

constexpr const char* G1_LOGGER = "g1";


class g1 : public GBase<g1> {
public:

	int object1 = 2;

	explicit g1(std::shared_ptr<GOptions> gopt) : GBase(gopt, G1_LOGGER) {
		log->info(0, "hello derived class ", SFUNCTION_NAME);
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

	return EXIT_SUCCESS;

}
