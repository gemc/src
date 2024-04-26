/**
 * \mainpage
 * \section main Overview
 *
 * The goptions framework provides command line and/or JSON file parsing of user defined options.\n
 * The JSON files (steering cards) have the extension <i>".jcard"</i> and are referred to as <i>jcards</i>.\n\n
 *
 * \subsection subsection0 Switch
 * A switch is a boolean variable, initialized to \"false\", that is switched true by command line. \n
 * For example, the switch \"gui\" is true if the command lne contains:\n
 * <pre>	-gui</pre>
 *
* \subsection subsection1 Simple Option
 * A simple option has a int, float, double or string value associated with it, and looks like this in the jcard:\n
 * ~~~
 *	"runno":  12
 * ~~~
 * and the corresponding command line option is:\n
 * <pre>	-runno=11  </pre>
 *
 * \subsection subsection2 Structured Option
 * A structured option has multiple objects associated with a tag. In the jcard:\n
 * ~~~
 * "beam": {
 * 	"vertex": "0, 0, -3, cm",
 * 	"value": "electron, 4*GeV, 20*deg, 180*deg"
 * }
 * ~~~
 * The corresponding  command line option is:\n
 * <pre>	-beam={ vertex: 0, 0, -3, cm; mom: electron, 4*GeV, 20*deg, 180*deg; }  </pre>
 * Notice the absence of quotes in the command line: the colons delimit the values, the semi-colons delimit the fields.
 * \n
 *
 * \subsection subsection3 Goptions  Main Features
 * * Goptions are dynamically added to an executable by a framework or plugin.
 * * Superposition of  command line / jcards option values.
 * * Mechanism to project options onto user defined structure/class.
 * * Mechanism to import child jcards.
 * * JSON output of user selections.
 * * HTML output of options help.
 *
 * \subsection subsection4 C++ User Interface
 * The users build his/her own GOptions class by calling the constructor:
 * <pre> GOptions(argc, argv, defineOptions())</pre>
 *
 * \param "argc, argv" passed from "main"
 * \param defineOptions() is the function building the options map.
 *
 * Check the examples below to see how to define and use a few GOptions.
 *
 * \subsubsection subsubsection1 Projections Onto Structures
 * GOptions can be projected onto a user structures.
 *
 * \subsection subsection5 examples
 * An example of defineOptions() that creates two categories is in simpleExample.cc:
 * Running <i>example -h</i> will produce the following log:
 * ~~~
 *
 * Usage:
 *
 * > -h, -help, --help: print this message and exit.
 * > -helpToHtml:  print all available options in HTML format (options.html) and exit.
 *
 * ~~~
 * \subsection subsection6  JSON Library and validator
 *
 * The JSON parser used is in this project is https://github.com/nlohmann/json. It's included as a single hpp.\n
 *
 * A json validator can be found here: https://codebeautify.org/jsonvalidator
 *
 * \subsection cisubsection Continuous Integration
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n
*/

