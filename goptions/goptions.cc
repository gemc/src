// goptions 
#include "goptions.h"
#include "goptionsConventions.h"
#include "gversion.h"

// gemc
#include "gutilities.h"

// c++
#include <iostream>
#include <cstring>

using namespace std;

/**
 * @brief Constructs a GOptions object.
 *
 * This constructor performs the following tasks:
 * - Loads user-defined options.
 * - Defines built-in options and switches.
 * - Parses YAML configuration files.
 * - Processes command-line arguments (supporting both YAML–style and dot–notation).
 * - Saves the final configuration to a YAML file.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @param user_defined_options A GOptions object containing user-defined options.
 */
GOptions::GOptions(int argc, char *argv[], const GOptions &user_defined_options) {

	executableName = gutilities::getFileFromPath(argv[0]);
	executableCallingDir = gutilities::getDirFromPath(argv[0]);
	installDir = gutilities::gemc_root();
	cout << endl;

	// Add user-defined options.
	addGOptions(user_defined_options);

	// switches for all everyone
	defineSwitch("gui", "use Graphical User Interface");
	defineSwitch("i", "use interactive batch mode");
	defineOption(
			GVariable("conf_yaml", "saved_configuration", "the prefix for filename that store the used options"),
			"The default value appends \"_saved_configuration\" to the executable name.");

	// add test timeout for the tests
	defineOption(GVariable("tt", 500, "tests timeout (ms)"),
						  "Timeout in milliseconds for the code tests that have GUI. ");

	// version is a special option, not settable by the user
	// it is set by the gversion.h file
	// we add it here so it can be saved to the yaml file
	vector <GVariable> version = {
			{"release",      gversion,      "release version number"},
			{"release_date", grelease_date, "release date"},
			{"Reference",    greference,    "article reference"},
			{"Homepage",     gweb,          "homepage"},
			{"Author",       gauthor,       "author"}
	};
	defineOption(GVERSION_STRING, "version information", version, "Version information. Not settable by user.");

	string help = "Levels: \n \n";
	help += " - 0: (default) shush\n";
	help += " - 1: normal information\n";
	help += " - 2: detailed information\n \n";
	help += "Example: -verbosity.general=1 \n \n";
	help += "This option can be repeated.\n \n";
	defineOption("verbosity", "Sets the log verbosity for various classes", option_verbosity_names, help);


	help = "Debug information Types: \n \n";
	help += " - false: (default): do not print debug information\n";
	help += " - true: print debug information\n\n";
	help += "Example: -debug.general=true \n \n";
	help += "This option can be repeated.\n \n";
	defineOption("debug", "Sets the debug level for various classes", option_verbosity_names, help);

	// Process help/version command-line arguments.
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--h") == 0 ||
			strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "--help") == 0) {
			printHelp();
		} else if (strcmp(argv[i], "-hweb") == 0) {
			printWebHelp();
		} else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--v") == 0 ||
				   strcmp(argv[i], "-version") == 0 || strcmp(argv[i], "--version") == 0) {
			print_version();
			exit(EXIT_SUCCESS);
		} else if (strcmp(argv[i], "help") == 0) {
			printOptionOrSwitchHelp(argv[i + 1]);
			exit(EXIT_SUCCESS);
		}
	}

	// finds and parse the yaml files
	yaml_files = findYamls(argc, argv);
	for (auto &yaml_file: yaml_files) {
		cout << " Parsing " << yaml_file << endl;
		setOptionsValuesFromYamlFile(yaml_file);
	}

	// Parse command-line arguments (supports both standard YAML–style and dot–notation).
	for (int i = 1; i < argc; i++) {
		string candidate = argv[i];
		if (candidate.empty()) continue;
		if (find(yaml_files.begin(), yaml_files.end(), candidate) != yaml_files.end()) continue;
		if (candidate[0] == '-') {
			string argStr = candidate.substr(1);
			size_t eqPos = argStr.find('=');
			if (eqPos != string::npos) {
				string keyPart = argStr.substr(0, eqPos);
				string valuePart = argStr.substr(eqPos + 1);

				// Strip outer quotes if present (e.g., -gstreamer="[...]")
				if (!valuePart.empty() && valuePart.front() == '"' && valuePart.back() == '"') {
					valuePart = valuePart.substr(1, valuePart.length() - 2);
				}

				size_t dotPos = keyPart.find('.');
				if (dotPos != string::npos) {
					// Dot–notation detected (e.g. "debug.general=true")
					string mainOption = keyPart.substr(0, dotPos);
					string subOption = keyPart.substr(dotPos + 1);
					if (doesOptionExist(mainOption)) {
						auto it = getOptionIterator(mainOption);
						it->set_sub_option_value(subOption, valuePart);
					} else {
						cerr << "The option " << mainOption << " is not known to this system." << endl;
						exit(EC__NOOPTIONFOUND);
					}
				} else {
					// Standard option syntax.
					if (doesOptionExist(keyPart)) {
						setOptionValuesFromCommandLineArgument(keyPart, valuePart);
					} else {
						cerr << "The option " << keyPart << " is not known to this system." << endl;
						exit(EC__NOOPTIONFOUND);
					}
				}
			} else {
				// Treat as a switch.
				const string& possibleSwitch = argStr;
				if (switches.find(possibleSwitch) != switches.end()) {
					switches[possibleSwitch].turnOn();
				} else {
					cerr << "The switch " << possibleSwitch << " is not known to this system." << endl;
					exit(EC__NOOPTIONFOUND);
				}
			}
		} else {
			cerr << "The command-line argument \"" << candidate << "\" is not valid." << endl;
			exit(EC__NOOPTIONFOUND);
		}
	}

	// Always print version information.
	print_version();

	// Save the final configuration to a YAML file.
	string yamlConf_filename = executableName + "." + getScalarString("conf_yaml") + ".yaml";
	cout << " Saving options to " << yamlConf_filename << endl << endl;
	yamlConf = new std::ofstream(yamlConf_filename);
	saveOptions();
}

/**
 * @brief Defines and adds a command-line switch.
 *
 * @param name The name of the switch.
 * @param description The description of the switch.
 */
void GOptions::defineSwitch(const std::string &name, const std::string &description) {
	if (switches.find(name) == switches.end()) {
		switches[name] = GSwitch(description);
	} else {
		std::cerr << FATALERRORL << "The " << YELLOWHHL << name << RSTHHR
				  << " switch is already present." << std::endl;
		exit(EC__DEFINED_SWITCHALREADYPRESENT);
	}
}

/**
 * @brief Defines and adds a scalar option.
 *
 * @param gvar The GVariable representing the option.
 * @param help Help text for the option.
 */
void GOptions::defineOption(const GVariable &gvar, const std::string &help) {
	if (doesOptionExist(gvar.name)) {
		std::cerr << FATALERRORL << "The " << YELLOWHHL << gvar.name << RSTHHR
				  << " option is already present." << std::endl;
		exit(EC__DEFINED_OPTION_ALREADY_PRESENT);
	} else {
		goptions.emplace_back(gvar, help);
	}
}

/**
 * @brief Defines and adds a structured option.
 *
 * @param name The name of the option.
 * @param description Summary description of the option.
 * @param g_vars Vector of GVariable objects representing the option elements.
 * @param help Help text for the option.
 */
void GOptions::defineOption(const std::string &name, const std::string &description, const std::vector <GVariable> &g_vars,
							const std::string &help) {
	if (doesOptionExist(name)) {
		std::cerr << FATALERRORL << "The " << YELLOWHHL << name << RSTHHR
				  << " option is already present." << std::endl;
		exit(EC__DEFINED_OPTION_ALREADY_PRESENT);
	} else {
		goptions.emplace_back(name, description, g_vars, help);
	}
}

/**
 * @brief Retrieves the value of a scalar integer option.
 *
 * @param tag The name of the option.
 * @return The integer value.
 */
int GOptions::getScalarInt(const std::string &tag) const {
	auto it = getOptionIterator(tag);
	if (it == goptions.end()) {
		cerr << FATALERRORL << "The option " << YELLOWHHL << tag << RSTHHR
			 << " was not found." << endl;
		exit(EC__NOOPTIONFOUND);
	}
	return it->value.begin()->second.as<int>();
}

/**
 * @brief Retrieves the value of a scalar double option.
 *
 * @param tag The name of the option.
 * @return The double value.
 */
double GOptions::getScalarDouble(const std::string &tag) const {
	auto it = getOptionIterator(tag);
	if (it == goptions.end()) {
		cerr << FATALERRORL << "The option " << YELLOWHHL << tag << RSTHHR
			 << " was not found." << endl;
		exit(EC__NOOPTIONFOUND);
	}
	return it->value.begin()->second.as<double>();
}

/**
 * @brief Retrieves the value of a scalar string option.
 *
 * @param tag The name of the option.
 * @return The string value.
 */
string GOptions::getScalarString(const std::string &tag) const {
	auto it = getOptionIterator(tag);
	if (it == goptions.end()) {
		cerr << FATALERRORL << "The option " << YELLOWHHL << tag << RSTHHR
			 << " was not found." << endl;
		exit(EC__NOOPTIONFOUND);
	}
	return it->value.begin()->second.as<string>();
}

/**
 * @brief Prints detailed help for a specific option or switch.
 *
 * @param tag The name of the option or switch.
 */
void GOptions::printOptionOrSwitchHelp(const std::string &tag) const {
	auto switchIt = switches.find(tag);
	if (switchIt != switches.end()) {
		cout << KGRN << "-" << tag << RST << ": " << switchIt->second.getDescription() << endl << endl;
		cout << TPOINTITEM << "Default value is " << (switchIt->second.getStatus() ? "on" : "off") << endl << endl;
		exit(EXIT_SUCCESS);
	}
	for (const auto &goption: goptions) {
		if (goption.name == tag) {
			goption.printHelp(true);
			exit(EXIT_SUCCESS);
		}
	}
	cerr << FATALERRORL << "The " << YELLOWHHL << tag << RSTHHR
		 << " option is not known to this system." << endl;
	exit(EC__NOOPTIONFOUND);
}

/**
 * @brief Finds YAML files specified by the command line.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @return A vector of YAML file paths.
 */
vector <string> GOptions::findYamls(int argc, char *argv[]) {
	vector <string> yaml_files;
	for (int i = 1; i < argc; i++) {
		string arg = argv[i];
		size_t pos = arg.find(".yaml");
		if (pos != string::npos) yaml_files.push_back(arg);
		pos = arg.find(".yml");
		if (pos != string::npos) yaml_files.push_back(arg);
	}
	return yaml_files;
}

// checks if the option exists
bool GOptions::doesOptionExist(const std::string &tag) const {

	// [&tag] ensures we're referencing the original tag passed to the function
	return std::any_of(goptions.begin(), goptions.end(),
	[&tag](const auto& option) {
		return option.name == tag;
	});
}

/**
 * @brief Parses and sets option values from a YAML file.
 *
 * @param yaml The YAML file path.
 */
void GOptions::setOptionsValuesFromYamlFile(const std::string &yaml) {
	YAML::Node config;
	try {
		config = YAML::LoadFile(yaml);
	} catch (YAML::ParserException &e) {
		cerr << FATALERRORL << "Error parsing " << YELLOWHHL << yaml << RSTHHR
			 << " yaml file." << endl;
		cerr << e.what() << endl;
		cerr << "Try validating the yaml file with an online yaml validator, e.g., https://www.yamllint.com" << endl;
		exit(EC__YAML_PARSING_ERROR);
	}

	for (auto it = config.begin(); it != config.end(); ++it) {
		auto option_name = it->first.as<std::string>();
		auto option_it = getOptionIterator(option_name);
		if (option_it == goptions.end()) {
			if (switches.find(option_name) == switches.end()) {
				cerr << FATALERRORL << "The option or switch " << YELLOWHHL << option_name << RSTHHR
					 << " is not known to this system." << endl;
				exit(EC__NOOPTIONFOUND);
			} else {
				switches[option_name].turnOn();
			}
		} else {
			YAML::NodeType::value type = it->second.Type();
			switch (type) {
				case YAML::NodeType::Scalar:
					option_it->set_scalar_value(it->second.as<std::string>());
					break;
				case YAML::NodeType::Sequence:
					option_it->set_value(it->second);
					break;
				case YAML::NodeType::Map:
					option_it->set_value(it->second);
					break;
				default:
					break;
			}
		}
	}
}

/**
 * @brief Parses and sets an option value from a command-line argument.
 *
 * @param optionName The name of the option.
 * @param possibleYamlNode The YAML-formatted value string.
 */
void GOptions::setOptionValuesFromCommandLineArgument(const std::string &optionName, const std::string &possibleYamlNode) {
	YAML::Node node = YAML::Load(possibleYamlNode);
	auto option_it = getOptionIterator(optionName);
	if (node.Type() == YAML::NodeType::Scalar) {
		option_it->set_scalar_value(possibleYamlNode);
	} else {
		option_it->set_value(node);
	}
}

/**
 * @brief Retrieves an iterator to the option with the specified name.
 *
 * @param name The name of the option.
 * @return An iterator to the option.
 */
std::vector<GOption>::iterator GOptions::getOptionIterator(const std::string &name) {
	return std::find_if(goptions.begin(), goptions.end(),
						[&name](GOption &option) { return option.name == name; });
}


/**
 * @brief Retrieves a const iterator to the option with the specified name.
 *
 * @param name The name of the option.
 * @return A const iterator to the option.
 */
std::vector<GOption>::const_iterator GOptions::getOptionIterator(const std::string &name) const {
	return std::find_if(goptions.begin(), goptions.end(),
						[&name](const GOption &option) { return option.name == name; });
}

/**
 * @brief Retrieves the status of the specified switch.
 *
 * @param tag The name of the switch.
 * @return True if the switch is on; false otherwise.
 */
bool GOptions::getSwitch(const std::string &tag) const {
	auto it = switches.find(tag);
	if (it != switches.end()) {
		return it->second.getStatus();
	} else {
		std::cerr << FATALERRORL << "The switch " << YELLOWHHL << tag << RSTHHR
				  << " was not found." << std::endl;
		exit(EC__NOOPTIONFOUND);
	}
}

/**
 * @brief Retrieves a map option’s value from within a YAML node.
 *
 * @param option_name The name of the option.
 * @param map_key The key to look up within the option.
 * @return The YAML::Node corresponding to the specified map key.
 */
YAML::Node GOptions::getOptionMapInNode(const string& option_name, const string& map_key) const {
	auto sequence_node = getOptionNode(option_name);
	for (auto seq_item : sequence_node) {
		for (auto map_item = seq_item.begin(); map_item != seq_item.end(); ++map_item) {
			if (map_item->first.as<string>() == map_key) {
				return map_item->second;
			}
		}
	}
	cerr << FATALERRORL << "The key " << YELLOWHHL << map_key << RSTHHR
		 << " was not found in " << YELLOWHHL << option_name << RSTHHR << endl;
	exit(EC__NOOPTIONFOUND);
}

/**
 * @brief Template function to retrieve a variable from a YAML node.
 *
 * @tparam T The type of the variable.
 * @param node The YAML node.
 * @param variable_name The name of the variable.
 * @param default_value The default value if the variable is not found.
 * @return The variable value.
 */
template<typename T>
T GOptions::get_variable_in_option(const YAML::Node &node, const std::string &variable_name, const T &default_value) {
	if (node[variable_name]) {
		return node[variable_name].as<T>();
	}
	return default_value;
}

// Explicit template instantiations.
template int GOptions::get_variable_in_option<int>(const YAML::Node &node, const std::string &variable_name, const int &default_value);
template double GOptions::get_variable_in_option<double>(const YAML::Node &node, const std::string &variable_name, const double &default_value);
template string GOptions::get_variable_in_option<string>(const YAML::Node &node, const std::string &variable_name, const string &default_value);
template bool GOptions::get_variable_in_option<bool>(const YAML::Node &node, const std::string &variable_name, const bool &default_value);

/**
 * @brief Retrieves the verbosity level for the specified tag.
 *
 * @param tag The name of the verbosity option.
 * @return The verbosity level as an integer.
 */
int GOptions::getVerbosityFor(const std::string &tag) const {
	YAML::Node verbosity_node = getOptionNode("verbosity");
	for (auto v : verbosity_node) {
		if (v.begin()->first.as<string>() == tag) {
			return v.begin()->second.as<int>();
		}
	}

	// not found. error
	std::cerr << KRED << " Invalid verbosity or debug requested: " << tag << RST << std::endl;
	exit(EC__NOOPTIONFOUND);
}

/**
 * @brief Retrieves the debug level for the specified tag.
 *
 * Accepts values as either booleans ("true"/"false") or integers.
 *
 * @param tag The name of the debug option.
 * @return The debug level as an integer.
 */
int GOptions::getDebugFor(const std::string &tag) const {
	YAML::Node debug_node = getOptionNode("debug");
	for (auto d : debug_node) {
		if (d.begin()->first.as<string>() == tag) {
			YAML::Node valNode = d.begin()->second;
			if (valNode.IsScalar()) {
				auto s = valNode.as<string>();
				if (s == "true") return 1;
				if (s == "false") return 0;
			}
			try {
				return valNode.as<int>();
			} catch (const YAML::BadConversion &) {
				std::cerr << "Invalid debug value for " << tag << std::endl;
				exit(EC__BAD_CONVERSION);
			}
		}
	}
	// not found. error
	std::cerr << KRED << " Invalid verbosity or debug requested: " << tag << RST << std::endl;
	exit(EC__NOOPTIONFOUND);}

/**
 * @brief Prints general help information to the console.
 */
void GOptions::printHelp() const {
	long int fill_width = string(HELPFILLSPACE).size() + 1;
	cout.fill('.');
	cout << KGRN << KBOLD << " " << executableName << RST << " [options] [yaml files]" << endl << endl;
	cout << " Switches: " << endl << endl;
	for (auto &s : switches) {
		string help = "-" + s.first + RST + " ";
		cout << KGRN << " " << left;
		cout.width(fill_width);
		cout << help;
		cout << ": " << s.second.getDescription() << endl;
	}
	cout << endl;
	cout << " Options: " << endl << endl;
	for (auto &option : goptions) {
		option.printHelp(false);
	}
	cout << endl;
	cout << endl << " Help / Search / Introspection: " << endl << endl;
	vector<string> helps = {
			string("-h, --h, -help, --help") + RST,
			string("print this help and exit"),
			string("-hweb") + RST,
			string("print this help in web format and exit"),
			string("-v, --v, -version, --version") + RST,
			string("print the version and exit\n"),
			string("help <value>") + RST,
			string("print detailed help for option <value> and exit"),
			string("search <value>") + RST,
			string("list all options containing <value> in the description and exit\n")
	};
	unsigned half_help = helps.size() / 2;
	for (unsigned i = 0; i < half_help; i++) {
		cout << KGRN << " " << left;
		cout.width(fill_width);
		cout << helps[i * 2] << ": " << helps[i * 2 + 1] << endl;
	}
	cout << endl;
	cout << " Note: command line options overwrite YAML file(s)." << endl << endl;
	exit(EXIT_SUCCESS);
}



/**
 * @brief Prints web-formatted help information.
 */
void GOptions::printWebHelp() const {
	exit(EXIT_SUCCESS);
}

/**
 * @brief Saves all switches and options to a YAML configuration file.
 */
void GOptions::saveOptions() const {
	for (auto &s : switches) {
		string status = s.second.getStatus() ? "true" : "false";
		*yamlConf << s.first + ": " + status << "," << endl;
	}
	for (const auto &option : goptions) {
		option.saveOption(yamlConf);
	}
	yamlConf->close();
}

/**
 * @brief Prints the version information of the executable.
 */
void GOptions::print_version() {
	string asterisks = "*******************************************************************";
	cout << endl << asterisks << endl;
	cout << " " << KGRN << KBOLD << executableName << RST << "  version: " << KGRN << gversion << RST << endl;
	cout << " Called from: " << KGRN <<  executableCallingDir << RST << endl;
	cout << " Executed from: " << KGRN <<  installDir << "/bin" << RST << endl;
	cout << " Released on: " << KGRN << grelease_date << RST << endl;
	cout << " GEMC Reference: " << KGRN << greference << RST << endl;
	cout << " GEMC Homepage: " << KGRN << gweb << RST << endl;
	cout << " Author: " << KGRN << gauthor << RST << endl << endl;
	cout << asterisks << endl << endl;
}

/**
 * @brief Overloaded operator to add options and switches from one GOptions object to another.
 *
 * @param gopts The original GOptions object.
 * @param goptions_to_add The GOptions object to add.
 * @return A reference to the updated GOptions object.
 */
GOptions &operator+=(GOptions &gopts, const GOptions &goptions_to_add) {
	gopts.addGOptions(goptions_to_add);
	return gopts;
}