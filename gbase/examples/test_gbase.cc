
#include "gbase.h"

class g1 : public GBase<g1> {
public:
	int object1 = 2;

	explicit g1(std::shared_ptr<GOptions> gopt) : GBase(gopt) {
		log->info(0, "hello derived class ", SFUNCTION_NAME);
	}

	~g1() = default;
};

// returns this example options
GOptions defineOptions() {

	GOptions goptions("g1");

	// command line switch
	goptions.defineSwitch("light", "a switch, this is just an example.");

	return goptions;
}

int main(int argc, char *argv[]) {

	auto gopts = std::make_shared<GOptions>(argc, argv, defineOptions());


	g1 obj(gopts);

	return EXIT_SUCCESS;

}
