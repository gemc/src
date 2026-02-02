#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * @file define_options.cc
 * @brief Example program showing how to define and use \ref GOptions : .
 *
 * @details
 * This example demonstrates:
 * - defining a switch with \ref GOptions::defineSwitch "defineSwitch()" ,
 * - defining scalar options with \ref GOptions::defineOption "defineOption()" ,
 * - defining a structured option schema (sequence of maps),
 * - constructing a parsing \ref GOptions : instance and reading values back via
 *   \ref GOptions::getScalarInt "getScalarInt()" .
 *
 * The entire file is guarded by \c DOXYGEN_SHOULD_SKIP_THIS so it does not appear in the
 * generated API reference unless desired.
 */

// goptions
#include "goptions.h"

// c++
#include <iostream>

using namespace std;

/**
 * @brief Builds and returns a set of example options.
 *
 * @details
 * The returned \ref GOptions : instance contains only definitions (no parsing). It is intended
 * to be passed to the parsing constructor:
 * \code{.cpp}
 * GOptions(argc, argv, defineOptions())
 * \endcode
 *
 * @return A \ref GOptions : object populated with example switches/options.
 */
GOptions defineOptions() {
	GOptions goptions;
	string   help;

	// command line switch
	goptions.defineSwitch("log", "a switch, this is just an example.");

	// runno: 12,
	help = "Example: -runno=12\n";
	goptions.defineOption(GVariable("runno", 1, "sets the run number"), help);

	// nthreads: 8
	help = "Maximum number of threads to use.\n";
	help += "If the number of cores available ncores is less then nthreads, use ncores instead.\n";
	help += "If not set, use all available threads. 0: use all threads\n";
	help += "Example: -nthreads=12  : . \n";
	goptions.defineOption(GVariable("nthreads", 0, "maximum number of threads to use"), help);

	vector<GVariable> gparticle = {
		{"name", goptions::NODFLT, "particle name"},
		{"multiplicity", 1, "number of particles per event"},
		{"p", goptions::NODFLT, "momentum"},
		{"theta", "0*degrees", "polar angle"},
		{"delta_theta", 0, "Particle polar angle range, centered on theta. Default: 0"},

	};

	help = "Example to add three particles, one electron and two protons, identical except spread in theta: \n \n";
	help +=
		"-gparticle=\"[{pname: e-, p: 2300, theta: 23.0}, {pname: proton, multiplicity: 2, p: 1200, delta_theta: 10}]\"\n";
	goptions.defineOption("gparticle", "define the generator particle(s)", gparticle, help);

	return goptions;
}

// Define options
int main(int argc, char* argv[]) {
	// Construct a parsing instance: this will parse YAML files and command-line arguments immediately.
	auto gopts = std::make_shared<GOptions>(argc, argv, defineOptions());

	// Typed access: retrieve a scalar integer option.
	cout << " > Nthreads: " << gopts->getScalarInt("nthreads") << endl;

	return EXIT_SUCCESS;
}

#endif
