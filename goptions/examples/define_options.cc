/**
 * @file define_options.cc
 * @brief Example program showing how to define and use \ref GOptions : .
 *
 * @details
 * This example demonstrates an end-to-end pattern commonly used in GEMC-style modules:
 * 1. Build a **definition-only** \ref GOptions : instance (no parsing yet).
 * 2. Construct a **parsing** \ref GOptions : instance in `main()` using:
 *    \ref GOptions::GOptions "GOptions(argc, argv, user_defined_options)" .
 * 3. Retrieve resolved values with typed getters such as
 *    \ref GOptions::getScalarInt "getScalarInt()" .
 *
 * It also shows:
 * - Defining a boolean switch with \ref GOptions::defineSwitch "defineSwitch()" .
 * - Defining scalar options with \ref GOptions::defineOption "defineOption()" .
 * - Defining a structured option schema (sequence of maps) and providing a usable command-line example.
 *
 */

// goptions
#include "goptions.h"

// c++
#include <iostream>

using namespace std;

/**
 * @brief Builds and returns a set of example options (definitions only).
 *
 * @details
 * This function intentionally does **not** parse any inputs. Instead, it returns a \ref GOptions :
 * populated with registrations:
 * - switches (presence-based booleans),
 * - scalar options,
 * - structured option schemas.
 *
 * The returned object is then passed into the parsing constructor:
 * \code{.cpp}
 * auto gopts = std::make_shared<GOptions>(argc, argv, defineOptions());
 * \endcode
 *
 * This pattern allows multiple components (modules/plugins) to contribute their own option definitions
 * and then combine them before parsing.
 *
 * What this example defines:
 * - Switch `log`: dummy switch that can be toggled by `-log`.
 * - Scalar `runno`: an integer option set via `-runno=<value>`.
 * - Scalar `nthreads`: an integer option controlling thread usage, with `0` meaning "use all cores".
 * - Structured `gparticle`: a schema representing generator particles as a sequence of maps, including
 *   mandatory keys flagged using \c goptions::NODFLT.
 *
 * @return A \ref GOptions : object populated with example switches and options.
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
	help += "Example: -nthreads=12 : . \n";
	goptions.defineOption(GVariable("nthreads", 0, "maximum number of threads to use"), help);

	vector<GVariable> gparticle = {
		{"name", goptions::NODFLT, "particle name"},
		{"multiplicity", 1, "number of particles per event"},
		{"p", goptions::NODFLT, "momentum"},
		{"theta", "0*degrees", "polar angle"},
		{"delta_theta", 0, "Particle polar angle range, centered on theta. Default: 0"},
	};

	help = "Example to add three particles, one electron and two protons, identical except spread in theta : \n \n";
	help +=
		"-gparticle=\"[{pname: e-, p: 2300, theta: 23.0}, {pname: proton, multiplicity: 2, p: 1200, delta_theta: 10}]\"\n";
	goptions.defineOption("gparticle", "define the generator particle(s)", gparticle, help);

	return goptions;
}

/**
 * @brief Example entry point: parse inputs and read back resolved values.
 *
 * @details
 * This `main()` illustrates the common consumption flow:
 * 1. Construct a parsing \ref GOptions : instance using `argc/argv` and the definitions provided by
 *    defineOptions().
 * 2. Query values from the resolved configuration.
 *
 * Example invocations:
 * - Scalar option from command line:
 *   \code{.sh}
 *   define_options -nthreads=8
 *   \endcode
 * - Dot-notation (if supported by the target option; shown here for illustration):
 *   \code{.sh}
 *   define_options -verbosity.general=1
 *   \endcode
 * - Structured option payload (quoted so the shell does not split YAML-like characters):
 *   \code{.sh}
 *   define_options -gparticle="[{name: e-, p: 2300, theta: 23.0}]"
 *   \endcode
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @return Process exit code (\c EXIT_SUCCESS on success).
 */
int main(int argc, char* argv[]) {
	// Construct a parsing instance: this will parse YAML files and command-line arguments immediately.
	auto gopts = std::make_shared<GOptions>(argc, argv, defineOptions());

	// Typed access: retrieve a scalar integer option.
	cout << " > Nthreads: " << gopts->getScalarInt("nthreads") << endl;

	return EXIT_SUCCESS;
}

