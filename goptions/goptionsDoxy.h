/**
 * \mainpage
 * \section main Overview
 *
 * GOptions provides an unified framework to parse command-line arguments
 * and YAML files. The main components are: GSwitch and GOption
 *
 * \subsection subsection0 GSwitches
 * A GSwitches is struct with a boolean flag and a description. It is initialized to `false` by default and can be toggled to `true` by specifying it on the command line.
 * For example, the switch "gui" is activated if the command line contains:
 * ~~~sh
 * -gui
 * ~~~
 *
 * \subsection subsection1 Simple GOption
 * A simple GOption is associated with a single value, which can be an integer, double, or string.
 * It is represented as follows in a YAML configuration file:
 * ~~~yaml
 * runno: 12
 * ~~~
 * The corresponding command-line option would be:
 * ~~~sh
 * -runno=12
 * ~~~
 *
 * \subsection subsection2 Structured GOption
 * A structured GOption consists of multiple key-value pairs within a single tag. An example from a YAML file:
 * ~~~yaml
 * gparticle:
 *  - name: e-
 *    p: 1500
 *    theta: 23.0
 *    multiplicity: 4
 * ~~~
 * The equivalent command-line option:
 * ~~~sh
 * -gparticle="[{name: e-, p: 1500, theta: 23.0, multiplicity: 4}]"
 * ~~~
 * Note the need of quotes in the command line to define.
 *
 * \subsection subsection3 GOptions Main Features
 * * Add options and switches to an executable via framework or plugins.
 * * Merging of command-line options with YAML file values. Command line options override YAML values.
 * * Import child YAML files for modular configuration.
 * * YAML output for all user-selected and default options.
 * * Automatic versioning and formatted help.
 *
 * \subsection subsection4 C++ User Interface
 * Users can instantiate the GOptions class by calling its constructor:
 * <pre> GOptions(argc, argv, defineOptions()) </pre>
 *
 * \param argc Number of command-line arguments passed from `main`.
 * \param argv Array of command-line arguments passed from `main`.
 * \param defineOptions Function that constructs and returns a GOptions object.
 *
 *
 * The defineOptions function creates and returns an instance of GOptions with predefined command line switches and options.
 *
 * Example with these options and switches:
 * - A switch `log` to enable logging.
 * - An integer option `runno` to set the run number with a default value.
 * - An integer option `nthreads` to set the number of threads to use, with a default that uses all available threads.
 * - A structured option `gparticle` to define generator particles with attributes such as name, multiplicity, momentum, and angles.
 *
 * \code
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
 *         {"name",  * goptions::NODFLT, "Particle name"},
 *         {"multiplicity", 1,                "Number of particles per event"},
 *         {"p",     * goptions::NODFLT, "Momentum"},
 *         {"theta",        "0*degrees",      "Polar angle"},
 *         {"delta_theta",  0,                "Particle polar angle range, centered on theta. Default: 0"},
 *     };
 *
 *     help = "Example to add three particles, one electron and two protons, identical except spread in theta:\n\n";
 *     help += "-gparticle=\"[{name: e-, multiplicity: 1, p: 2300, theta: 23.0}, {name: proton, multiplicity: 2, p: 1200, theta: 14.0, delta_theta: 10}]\"\n";
 *     goptions.defineOption("gparticle", "Define the generator particle(s)", gparticle, help);
 *
 *     return goptions;
 * }
 * \endcode
 *
 * \subsubsection subsubsection1 Adding Options and Switches from a framework or plugin
 *
 * Each framework or plugin can define its own options and switches. The user can then merge these with the main executable's.
 * For example, in the gemc defineOptions function, the Goptions defined in the external libraries defineOptions
 * and added, as in the code below:
 * 
 * \code
 * goptions += eventDispenser::defineOptions();
 * goptions += g4display::defineOptions();
 * goptions += g4system::defineOptions();
 * goptions += gfield::defineOptions();
 * goptions += gparticle::defineOptions();
 * goptions += gphysics::defineOptions();
 * goptions += gstreamer::defineOptions();
 * goptions += gsystem::defineOptions();
 *
 * \endcode
 * 
 * \subsection subsection6 YAML Library and Validator
 *
 * The YAML parser used in this project is from the [yaml-cpp](https://github.com/jbeder/yaml-cpp) library.
 * It is included as a dependency and facilitates parsing complex YAML configurations.
 *
 * \subsection cisubsection Continuous Integration
 * The GOptions framework is continuously integrated and tested to ensure stability and reliability across updates.
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n
 */
