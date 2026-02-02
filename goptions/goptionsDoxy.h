#pragma once

/**
 * \mainpage
 * \section main Overview
 *
 * \ref GOptions provides a unified framework to parse command-line arguments and YAML files.
 * The primary building blocks are \ref GSwitch and \ref GOption.
 *
 * \subsection Subsection0 Switches
 * A \ref GSwitch is a lightweight boolean flag with a description.
 * It is initialized to `false` by default and can be toggled to `true` by specifying it on the command line.
 * For example, the switch `gui` is activated if the command line contains:
 * ~~~sh
 * -gui
 * ~~~
 *
 * \subsection Subsection1 Scalar options
 * A scalar \ref GOption is associated with a single value (integer, double, string, etc.) stored as a YAML scalar.
 * In a YAML configuration file:
 * ~~~yaml
 * runno: 12
 * ~~~
 * The corresponding command-line syntax is:
 * ~~~sh
 * -runno=12
 * ~~~
 *
 * \subsection Subsection2 Structured options
 * A structured \ref GOption groups multiple key/value pairs under one option name.
 * Example YAML:
 * ~~~yaml
 * gparticle:
 *  - name: e-
 *    p: 1500
 *    theta: 23.0
 *    multiplicity: 4
 * ~~~
 * Equivalent command-line option:
 * ~~~sh
 * -gparticle="[{name: e-, p: 1500, theta: 23.0, multiplicity: 4}]"
 * ~~~
 * Note the need for quotes on the command line when passing structured YAML.
 *
 * \subsection Subsection3 Dot-notation for sub-options
 * Some structured options are updated using dot-notation:
 * ~~~sh
 * -verbosity.general=1
 * -debug.general=true
 * ~~~
 * This updates a single subkey within the structured option, via \ref GOption::set_sub_option_value "set_sub_option_value()".
 *
 * \subsection Subsection4 Main features
 * * Add options and switches to an executable via frameworks or plugins.
 * * Merge command-line options with YAML file values (command line overrides YAML).
 * * YAML output for all user-selected and default options.
 * * Automatic versioning and formatted help output.
 *
 * \subsection Subsection5 C++ user interface
 * Users instantiate the parsing \ref GOptions object by calling its constructor:
 * \code{.cpp}
 * GOptions(argc, argv, defineOptions())
 * \endcode
 *
 * \param argc Number of command-line arguments passed from `main`.
 * \param argv Array of command-line arguments passed from `main`.
 * \param defineOptions Function that constructs and returns a \ref GOptions object with definitions.
 *
 * The \c defineOptions() function creates and returns an instance of \ref GOptions with predefined
 * command-line switches and options.
 *
 * Example with these options and switches:
 * - A switch `log` to enable logging.
 * - An integer option `runno` to set the run number with a default value.
 * - An integer option `nthreads` to set the number of threads to use, defaulting to all available threads.
 * - A structured option `gparticle` to define generator particles with attributes such as name,
 *   multiplicity, momentum, and angles.
 *
 * \code{.cpp}
 * GOptions defineOptions() {
 *     GOptions goptions;
 *     string help;
 *
 *     // Command line switch
 *     goptions.defineSwitch("log", "A switch, this is just an example.");
 *
 *     // Option for run number
 *     help = "Example: -runno=12\n";
 *     goptions.defineOption(GVariable("runno", 1, "Sets the run number"), help);
 *
 *     // Option for number of threads
 *     help = "If not set, use all available threads. 0: use all threads\n";
 *     help += "Example: -nthreads=12\n";
 *     goptions.defineOption(GVariable("nthreads", 0, "Number of threads"), help);
 *
 *     // Vector of GVariable for structured option gparticle
 *     vector <GVariable> gparticle = {
 *         {"name",  goptions::NODFLT, "Particle name"},
 *         {"multiplicity", 1,                "Number of particles per event"},
 *         {"p",     goptions::NODFLT, "Momentum"},
 *         {"theta",        "0*degrees",      "Polar angle"},
 *         {"delta_theta",  0,                "Particle polar angle range, centered on theta. Default: 0"},
 *     };
 *
 *     help = "Example to add three particles, one electron and two protons, identical except spread in theta:\n\n";
 *     help += "-gparticle=\"[{name: e-, p: 2300, theta: 23.0}, {name: proton, multiplicity: 2, p: 1200, delta_theta: 10}]\"\n";
 *     goptions.defineOption("gparticle", "Define the generator particle(s)", gparticle, help);
 *
 *     return goptions;
 * }
 * \endcode
 *
 * \subsection subsection6 Adding options and switches from frameworks/plugins
 * Each framework or plugin can define its own options and switches. The user can merge these with the main executable's
 * definitions using the \ref operator+=  which internally uses \ref GOptions::addGOptions "addGOptions()".
 * For example
 *
 * \code{.cpp}
 * goptions += eventDispenser::defineOptions();
 * goptions += g4display::defineOptions();
 * goptions += g4system::defineOptions();
 * goptions += gfield::defineOptions();
 * goptions += gparticle::defineOptions();
 * goptions += gphysics::defineOptions();
 * goptions += gstreamer::defineOptions();
 * goptions += gsystem::defineOptions();
 * \endcode
 *
 * \subsection subsection7 YAML library and validation
 * The YAML parser used in this project is from the yaml-cpp library (https://github.com/jbeder/yaml-cpp).
 * It is included as a dependency and facilitates parsing complex YAML configurations.
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n
 */
