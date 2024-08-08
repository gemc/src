// goptions 
#include "goptions.h"
#include "goptionsConventions.h"
#include "gversion.h"

// gemc
#include "gutilities.h"

// c++
#include <iostream>
// c style string
#include <cstring>

using namespace std;


// constructor:
// - load user defined options, add goptions options
// - parse the yaml files
// - parse the command line options
GOptions::GOptions(int argc, char *argv[], const GOptions &user_defined_options) {

    executableName = gutilities::getFileFromPath(argv[0]);

    cout << endl;

    // copy the user defined options maps and switches onto ours
    addGOptions(user_defined_options);

    // switches for all everyone
    defineSwitch("gui", "use Graphical User Interface");
    defineOption(GVariable("conf_yaml", "saved_configuration", "the yaml filename prefix where all used options are saved"),
                 "The default value appends \"_saved_configuration\" to the executable name.");

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


    vector <GVariable> verbosity = {
            {"ghits",            0, "ghits verbosity"},
            {"gmaterials",       0, "gmaterials verbosity"},
            {"gevent_dispenser", 0, "event dispenser verbosity"},
            {"grun",             0, "run verbosity"},
            {"g4display",        0, "g4display verbositythe "},
            {"gsystem",          0, "gsystem verbositythe "},
            {"gfield",           0, "general fields verbosity"},
            {"g4system",         0, "g4system verbosity"},
            {"gparticle",        0, "gparticle verbosity"},
            {"gphysics",         0, "gphysics verbosity"},
            {"gstreamer_ev",     0, "gstreamer event verbosity"},
            {"gstreamer_fr",     0, "gstreamer frame verbosity"},
            {"gsensitivity",     0, "sensitivity verbosity"},
            {"general",          0, "general verbosity"},
            {"event",            0, "event verbosity"},
    };

    string help = "Levels: \n \n";
    help += "0: shush\n";
    help += "1: summaryt\n";
    help += "2: details\n";
    help += "3: everything\n \n";
    help += "Example: -verbosity=\"[{gsystem: 3}, {grun: 1}]\" \n";
    defineOption("verbosity", "Sets the log verbosity for various categories", verbosity, help);


    // parsing command line to check for help
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--h") == 0 || strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "--help") == 0) {
            printHelp();
        } else if (strcmp(argv[i], "-hweb") == 0) {
            printWebHelp();
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--v") == 0 || strcmp(argv[i], "-version") == 0 || strcmp(argv[i], "--version") == 0) {
            print_version();
            exit(EXIT_SUCCESS);
        } else if (strcmp(argv[i], "help") == 0) {
            printOptionOrSwitchHelp(argv[i + 1]);
            exit(EXIT_SUCCESS);
        }
    }


    // finds the yaml files
    yaml_files = findYamls(argc, argv);

    // parse the yaml files
    for (auto &yaml_file: yaml_files) {
        cout << " Parsing " << yaml_file << endl;
        setOptionsValuesFromYamlFile(yaml_file);
    }


    // parse command lines
    // check that every option passed is either a switch, an option or a yaml file
    for (int i = 1; i < argc; i++) {
        string candidate = string(argv[i]);
        // empty string
        if (candidate == "") continue;

        if (find(yaml_files.begin(), yaml_files.end(), candidate) == yaml_files.end()) {

            // if the command line is not a yaml file, check that is a valid
            // - switch: starts with a dash
            // - option: a dash followed by a string and an equal sign
            if (candidate[0] == '-') {

                // checking for a switch
                string possible_switch = candidate.substr(1, candidate.size() - 1);

                // switch found, turn it on
                if (switches.find(possible_switch) != switches.end()) {
                    switches[possible_switch].turnOn();
                } else {
                    // not a switch, check if it is an option
                    // checking if '-' is present
                    if (possible_switch.find("=") != string::npos) {

                        string possible_option = possible_switch.substr(0, candidate.find("=") - 1);

                        // option found, parse it
                        if (doesOptionExist(possible_option)) {
                            string possible_yaml_node = possible_switch.substr(candidate.find("="), candidate.size() - 1);
                            setOptionValuesFromCommandLineArgument(possible_option, possible_yaml_node);
                        } else {
                            // option not found
                            cerr << FATALERRORL << "the " << YELLOWHHL << candidate << RSTHHR << " option is not known to this system. " << endl;
                            cerr << endl << "   " << executableName << " -h for help." << endl << endl;
                            gexit(EC__NOOPTIONFOUND);

                        }
                    } else {
                        // not a switch, not an option
                        cerr << FATALERRORL << YELLOWHHL << candidate << RSTHHR << " is not a valid command line option or switch.  " << endl;
                        cerr << " Note: switches start with a dash; options start with a dash, and are followed by an equal sign and their desired value."
                             << endl;
                        cerr << endl << " Usage: " << endl << endl;
                        cerr << "   " << executableName << " [options] [yaml files]" << endl;
                        cerr << "   " << executableName << " -h for help." << endl << endl;
                        gexit(EC__NOOPTIONFOUND);
                    }
                }
            } else {
                // not a file, not a switch, not an option
                cerr << FATALERRORL << "the " << YELLOWHHL << candidate << RSTHHR << " command line is not known to this system. " << endl;
                cerr << endl << " Usage: " << endl << endl;
                cerr << "   " << executableName << " [options] [yaml files]" << endl;
                cerr << "   " << executableName << " -h for help." << endl << endl;
                gexit(EC__NOOPTIONFOUND);
            }
        }
    }

    // print version no matter what
    print_version();

    // save options to yaml
    string yamlConf_filename = executableName + "." + getScalarString("conf_yaml") + ".yaml";
    cout << " Saving options to " << yamlConf_filename << endl << endl;
    yamlConf =  new std::ofstream(yamlConf_filename);

    saveOptions();
}

// define and add a command line switch to the map of switches
void GOptions::defineSwitch(const std::string &name, const std::string &description) {
    if (switches.find(name) == switches.end()) {
        switches[name] = GSwitch(description);
    } else {
        std::cerr << FATALERRORL << "the " << YELLOWHHL << name << RSTHHR << " switch is already present." << std::endl;
        gexit(EC__DEFINED_SWITCHALREADYPRESENT);
    }
}

// add a simple option to the map of options
void GOptions::defineOption(const GVariable &gvar, const std::string &help) {

    if (doesOptionExist(gvar.name)) {
        std::cerr << FATALERRORL << "the " << YELLOWHHL << gvar.name << RSTHHR << " option is already present." << std::endl;
        gexit(EC__DEFINED_OPTION_ALREADY_PRESENT);
    } else {
        goptions.push_back(GOption(gvar, help));
    }
}

// add a map option to the map of options
void GOptions::defineOption(const std::string &name, const std::string &description, const std::vector <GVariable> &g_vars, const std::string &help) {

    if (doesOptionExist(name)) {
        std::cerr << FATALERRORL << "the " << YELLOWHHL << name << RSTHHR << " option is already present." << std::endl;
        gexit(EC__DEFINED_OPTION_ALREADY_PRESENT);
    } else {
        goptions.push_back(GOption(name, description, g_vars, help));
    }
}

int GOptions::getScalarInt(const std::string &tag) const {
    auto it = getOptionIterator(tag);

    // if the option is not found, exit with error
    if (it == goptions.end()) {
        cerr << FATALERRORL << "The option " << YELLOWHHL << tag << RSTHHR << " was not found." << endl;
        gexit(EC__NOOPTIONFOUND);
    }

    return it->value.begin()->second.as<int>();
}

float GOptions::getScalarFloat(const std::string &tag) const {
    auto it = getOptionIterator(tag);

    // if the option is not found, exit with error
    if (it == goptions.end()) {
        cerr << FATALERRORL << "The option " << YELLOWHHL << tag << RSTHHR << " was not found." << endl;
        gexit(EC__NOOPTIONFOUND);
    }

    return it->value.begin()->second.as<float>();
}

double GOptions::getScalarDouble(const std::string &tag) const {
    auto it = getOptionIterator(tag);

    // if the option is not found, exit with error
    if (it == goptions.end()) {
        cerr << FATALERRORL << "The option " << YELLOWHHL << tag << RSTHHR << " was not found." << endl;
        gexit(EC__NOOPTIONFOUND);
    }

    return it->value.begin()->second.as<double>();
}

string GOptions::getScalarString(const std::string &tag) const {
    auto it = getOptionIterator(tag);

    // if the option is not found, exit with error
    if (it == goptions.end()) {
        cerr << FATALERRORL << "The option " << YELLOWHHL << tag << RSTHHR << " was not found." << endl;
        gexit(EC__NOOPTIONFOUND);
    }

    return it->value.begin()->second.as<string>();
}

void GOptions::printOptionOrSwitchHelp(const std::string& tag) const {
    // Check if the tag is a switch
    auto switchIt = switches.find(tag);
    if (switchIt != switches.end()) {
        cout << KGRN << "-" << tag << RST << ": " << switchIt->second.getDescription() << endl << endl;
        cout << TPOINTITEM << "Default value is " << (switchIt->second.getStatus() ? "on" : "off") << endl << endl;
        exit(EXIT_SUCCESS);
    }

    // Check if the tag is an option
    for (const auto& goption : goptions) {  // Use const auto& to avoid copying and ensure const-correctness
        if (goption.name == tag) {
            goption.printHelp(true);  // Assuming printHelp() is const-correct
            exit(EXIT_SUCCESS);
        }
    }

    // If not found, print error and exit
    cerr << FATALERRORL << "The " << YELLOWHHL << tag << RSTHHR << " option is not known to this system." << endl;
    gexit(EC__NOOPTIONFOUND);
}


// Finds the (first) configuration file (yaml or yml extensions).
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
bool GOptions::doesOptionExist(const std::string& tag) const {
    for (const auto& goption : goptions) {
        // Use const auto& to avoid copying and to ensure const-correctness
        if (goption.name == tag) {
            return true;
        }
    }
    return false;
}

void GOptions::setOptionsValuesFromYamlFile(const std::string& yaml) {

    YAML::Node config;
    try {
        config = YAML::LoadFile(yaml);

    } catch (YAML::ParserException &e) {
        cerr << FATALERRORL << "Error parsing " << YELLOWHHL << yaml << RSTHHR << " yaml file." << endl;
        cerr << e.what() << endl;
        cerr << "Try validating the yaml file with an online yaml validator, for example: https://www.yamllint.com" << endl;
        gexit(EC__YAML_PARSING_ERROR);
    }

    for (YAML::const_iterator it = config.begin(); it != config.end(); ++it) {

        string option_name = it->first.as<std::string>();
        auto option_it = getOptionIterator(option_name);

        if (option_it == goptions.end()) {
            if (switches.find(option_name) == switches.end()) {
                cerr << FATALERRORL << "The option or switch " << YELLOWHHL << option_name << RSTHHR << " is not known to this system." << endl;
                gexit(EC__NOOPTIONFOUND);
            } else {
                switches[option_name].turnOn();
            }
        } else {
            YAML::NodeType::value type = it->second.Type(); // Cache the type to avoid repeated calls

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

// parse a command line
void GOptions::setOptionValuesFromCommandLineArgument(const std::string& optionName, const std::string& possibleYamlNode) {
    YAML::Node node = YAML::Load(possibleYamlNode);

    auto option_it = getOptionIterator(optionName);

    if (node.Type() == YAML::NodeType::Scalar) {
        option_it->set_scalar_value(possibleYamlNode);
    } else {
        option_it->set_value(node);
    }
}

// Non-const version
std::vector<GOption>::iterator GOptions::getOptionIterator(const std::string& name) {
    return std::find_if(goptions.begin(), goptions.end(),
                        [&name](GOption& option) { return option.name == name; });
}

// Const version
std::vector<GOption>::const_iterator GOptions::getOptionIterator(const std::string& name) const {
    return std::find_if(goptions.begin(), goptions.end(),
                        [&name](const GOption& option) { return option.name == name; });
}


bool GOptions::getSwitch(const std::string &tag) const {
    // Use the find method to get an iterator to the switch
    auto it = switches.find(tag);

    // Check if the iterator is not at the end, indicating the switch was found
    if (it != switches.end()) {
        return it->second.getStatus();
    } else {
        std::cerr << FATALERRORL << "The switch " << YELLOWHHL << tag << RSTHHR << " was not found." << std::endl;
        gexit(EC__NOOPTIONFOUND);
    }
    return false; // This will never be reached due to gexit, but included for completeness
}

YAML::Node GOptions::getOptionMapInNode(string option_name, string map_key) {

    auto sequence_node = getOptionNode(option_name);

    for (auto seq_item: sequence_node) {
        for (auto map_item = seq_item.begin(); map_item != seq_item.end(); ++map_item) {
            if (map_item->first.as<string>() == map_key) {
                return map_item->second;
            }
        }
    }

    // if the key is not found, exit with error
    cerr << FATALERRORL << "The key " << YELLOWHHL << map_key << RSTHHR << " was not found in " << YELLOWHHL << option_name << RSTHHR << endl;
    gexit(EC__NOOPTIONFOUND);

    return sequence_node;
}

template<typename T>
T GOptions::get_variable_in_option(const YAML::Node &node, const string &variable_name, const T &default_value) {
    if (node[variable_name]) {
        return node[variable_name].as<T>();
    }
    return default_value;
}

// Explicit instantiations
template int GOptions::get_variable_in_option<int>(const YAML::Node &node, const std::string &variable_name, const int &default_value);

template float GOptions::get_variable_in_option<float>(const YAML::Node &node, const std::string &variable_name, const float &default_value);

template double GOptions::get_variable_in_option<double>(const YAML::Node &node, const std::string &variable_name, const double &default_value);

template string GOptions::get_variable_in_option<string>(const YAML::Node &node, const std::string &variable_name, const string &default_value);

template bool GOptions::get_variable_in_option<bool>(const YAML::Node &node, const std::string &variable_name, const bool &default_value);

int GOptions::getVerbosityFor(const std::string& tag) const{
    auto verbosity_node = getOptionNode("verbosity");

    for (auto v: verbosity_node) {
        if (v.begin()->first.as<string>() == tag) {
            return v.begin()->second.as<int>();
        }
    }

    return 0;
}


// print only the non default settings set by users
void GOptions::printHelp() const {

    long int fill_width = string(HELPFILLSPACE).size() + 1;
    cout.fill('.');

    cout << KGRN << KBOLD << " " << executableName << RST << " [options] [yaml files]" << endl << endl;
    cout << " Switches: " << endl << endl;

    // switches help, belongs here cause of the map key
    for (auto &s: switches) {
        string help = "-" + s.first + RST + " ";
        cout << KGRN << " " << left;
        cout.width(fill_width);
        cout << help;
        cout << ": " << s.second.getDescription() << endl;
    }
    cout << endl;

    cout << " Options: " << endl << endl;

    for (auto &option: goptions) {
        option.printHelp(false);
    }

    cout << endl;

    cout << endl << " Help / Search / Introspection: " << endl << endl;

    vector <string> helps = {
            string("-h, --h, -help, --help") + RST,
            string("print this help and exit"),
            string("-hweb") + RST,
            string("print this help in web format and exit"),
            string("-v, --v, -version, --version") + RST,
            string("print the version and exit\n"), string("help <value>") + RST,
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


    cout << " Note: command line options overwrite yaml file(s). " << endl << endl;

    exit(EXIT_SUCCESS);
}


// print only the non default settings set by users
void GOptions::printWebHelp() const {

    exit(EXIT_SUCCESS);
}


// print options and switches values
void GOptions::saveOptions() const {

    for (auto &s: switches) {
        string status = "false";
        if (s.second.getStatus()) status = "true";
        *yamlConf << s.first + ": " + status << "," << endl;
    }

    for (const auto& option: goptions) {
        option.saveOption(yamlConf);
    }

    yamlConf->close();
}

// introspection, add file option
void GOptions::print_version() {
    string asterisks = "**************************************************************";
    cout << endl << asterisks << endl;
    cout << " " << KGRN << KBOLD << executableName << RST << "  version: " << KGRN << gversion << RST << endl;
    cout << " Released on: " << KGRN << grelease_date << RST << endl;
    cout << " Reference: " << KGRN << greference << RST << endl;
    cout << " Homepage: " << KGRN << gweb << RST << endl;
    cout << " Author: " << KGRN << gauthor << RST << endl << endl;
    cout << asterisks << endl << endl;

}

// overloaded operator to add option vectors and switch maps
GOptions &operator+=(GOptions &gopts, const GOptions& goptions_to_add) {
    gopts.addGOptions(goptions_to_add);
    return gopts;
}
