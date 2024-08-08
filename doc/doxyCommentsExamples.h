// Make sure that the next strig after subsubsection or subsection is indexed correctly

/**
 * \mainpage
 * \section main Overview
 *
 * The goptions framework provides command line and JSON file parsing of user
 * defined options.\n\n
 *
 * \subsection subsection1 Simple Option
 * A simple command line option looks like:\n
 * <pre>	-runno=11  </pre>
 *
 * while the corresponding JSON syntax is:\n
 * ~~~
 *	"runno":  12
 * ~~~
 * \subsection subsection2 Structured Option
 * A structured option has this JSON syntax:\n
 * ~~~
 * "beam": {
 * 	"vertex": "0, 0, -3, cm",\n
 * 	"value": "electron, 4*GeV, 20*deg, 180*deg"
 * }
 * ~~~
 * and the corresponding  command line option is:\n
 * <pre>	-beam={ vertex: 0, 0, -3, cm; mom: electron, 4*GeV, 20*deg, 180*deg; }  </pre>
 * Notice the absence of quotes from the command line: the semi-colon delimits the option value.
 * \n
 *
 * \subsection subsection3 Goptions  Features
 * * Goptions can be dynamically added to an executable by a framework or plugin
 * * Superposition of  command line / steering card (JSON file with .jcard extension)
 * * JSON, HTML outputs for documentation
 * * Can import a jcard within another one (imports should be declared at the top of the jcard)
 * * Mechanism to project options onto user defined structure/class
 *
 * \subsection subsection4 C++ User Interface
 * The users defines his/her own GOption map by calling the GOptions constructor:
 * <pre> GOptions(argc, argv, defineOptions())</pre>
 *
 * \param "argc, argv" passed from "main"
 * \param defineOptions() is a function returning a map<string, GOption>
 *
 * Check the example below to see how to define a few GOptions.
 *
 * \subsection subsection5 example
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
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n
 */
