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
GOptions::GOptions(int argc, char *argv[], GOptions user_defined_options) {

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
            {"gfield",           0, "general fields verbositythe "},
            {"g4system",         0, "g4system verbositythe "},
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
            print_help();
        } else if (strcmp(argv[i], "-hweb") == 0) {
            print_web_help();
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--v") == 0 || strcmp(argv[i], "-version") == 0 || strcmp(argv[i], "--version") == 0) {
            print_version();
            exit(EXIT_SUCCESS);
        } else if (strcmp(argv[i], "help") == 0) {
            print_option_or_switch_help(argv[i + 1]);
            exit(EXIT_SUCCESS);
        }
    }


    // finds the yaml files
    yaml_files = find_yamls(argc, argv);

    // parse the yaml files
    for (auto &yaml_file: yaml_files) {
        cout << " Parsing " << yaml_file << endl;
        set_options_values_from_yaml_file(yaml_file);
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
                        if (does_option_exist(possible_option)) {
                            string possible_yaml_node = possible_switch.substr(candidate.find("="), candidate.size() - 1);
                            set_option_values_from_command_line_argument(possible_option, possible_yaml_node);
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
    string yaml_conf_filename = executableName + "." + getScalarString("conf_yaml") + ".yaml";
    cout << " Saving options to " << yaml_conf_filename << endl << endl;
    yaml_conf = new ofstream(yaml_conf_filename);

    save_options();
}

void GOptions::print_option_or_switch_help(string tag) {
    if (switches.find(tag) != switches.end()) {
        cout << KGRN << "-" << tag << RST << ": " << switches[tag].getDescription() << endl << endl;
        cout << TPOINTITEM << "Default value is " << (switches[tag].getStatus() ? "on" : "off") << endl << endl;
        exit(EXIT_SUCCESS);
    } else {
        for (auto &goption: goptions) {
            if (goption.name == tag) {
                goption.print_help(true);
                exit(EXIT_SUCCESS);
            }
        }
        cerr << FATALERRORL << "the " << YELLOWHHL << tag << RSTHHR << " option is not known to this system. " << endl;
        gexit(EC__NOOPTIONFOUND);
    }
}


// Finds the (first) configuration file (yaml or yml extensions).
vector <string> GOptions::find_yamls(int argc, char *argv[]) {
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
bool GOptions::does_option_exist(string tag) {
    for (auto &goption: goptions) {
        if (goption.name == tag) {
            return true;
        }
    }
    return false;
}

// add a command line switch to the map of switches
void GOptions::defineSwitch(string name, string description) {
    if (switches.find(name) == switches.end()) {
        switches[name] = GSwitch(description);
    } else {
        std::cerr << FATALERRORL << "the " << YELLOWHHL << name << RSTHHR << " switch is already present." << std::endl;
        gexit(EC__DEFINED_SWITCHALREADYPRESENT);
    }
}

// add a simple option to the map of options
void GOptions::defineOption(GVariable gvar, string help) {

    if (does_option_exist(gvar.name)) {
        std::cerr << FATALERRORL << "the " << YELLOWHHL << gvar.name << RSTHHR << " option is already present." << std::endl;
        gexit(EC__DEFINED_OPTION_ALREADY_PRESENT);
    } else {
        goptions.push_back(GOption(gvar, help));
    }
}


// add a map option to the map of options
void GOptions::defineOption(string name, string description, vector <GVariable> g_vars, string help) {

    if (does_option_exist(name)) {
        std::cerr << FATALERRORL << "the " << YELLOWHHL << name << RSTHHR << " option is already present." << std::endl;
        gexit(EC__DEFINED_OPTION_ALREADY_PRESENT);
    } else {
        goptions.push_back(GOption(name, description, g_vars, help));
    }
}

void GOptions::set_options_values_from_yaml_file(string yaml) {

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
        auto option_it = get_option_iterator(option_name);

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
void GOptions::set_option_values_from_command_line_argument(string option_name, string possible_yaml_node) {
    YAML::Node node = YAML::Load(possible_yaml_node);

    auto option_it = get_option_iterator(option_name);

    if (node.Type() == YAML::NodeType::Scalar) {
        option_it->set_scalar_value(possible_yaml_node);
    } else {
        option_it->set_value(node);
    }
}

// returns vector<GOption> iterator for option name
vector<GOption>::iterator GOptions::get_option_iterator(const string& name) {

    for (auto it = goptions.begin(); it != goptions.end(); ++it) {
        if (it->name == name) {
            return it;
        }
    }

    return goptions.end();
}

int GOptions::getScalarInt(string tag) {
    auto it = get_option_iterator(tag);

    // if the option is not found, exit with error
    if (it == goptions.end()) {
        cerr << FATALERRORL << "The option " << YELLOWHHL << tag << RSTHHR << " was not found." << endl;
        gexit(EC__NOOPTIONFOUND);
    }

    return it->value.begin()->second.as<int>();
}

float GOptions::getScalarFloat(string tag) {
    auto it = get_option_iterator(tag);

    // if the option is not found, exit with error
    if (it == goptions.end()) {
        cerr << FATALERRORL << "The option " << YELLOWHHL << tag << RSTHHR << " was not found." << endl;
        gexit(EC__NOOPTIONFOUND);
    }

    return it->value.begin()->second.as<float>();
}

double GOptions::getScalarDouble(string tag) {
    auto it = get_option_iterator(tag);

    // if the option is not found, exit with error
    if (it == goptions.end()) {
        cerr << FATALERRORL << "The option " << YELLOWHHL << tag << RSTHHR << " was not found." << endl;
        gexit(EC__NOOPTIONFOUND);
    }

    return it->value.begin()->second.as<double>();
}

string GOptions::getScalarString(string tag) {
    auto it = get_option_iterator(tag);

    // if the option is not found, exit with error
    if (it == goptions.end()) {
        cerr << FATALERRORL << "The option " << YELLOWHHL << tag << RSTHHR << " was not found." << endl;
        gexit(EC__NOOPTIONFOUND);
    }

    return it->value.begin()->second.as<string>();
}

bool GOptions::getSwitch(string tag) {
    if (switches.find(tag) != switches.end()) {
        return switches[tag].getStatus();
    } else {
        cerr << FATALERRORL << "The switch " << YELLOWHHL << tag << RSTHHR << " was not found." << endl;
        gexit(EC__NOOPTIONFOUND);
    }
    return false;
}


YAML::Node GOptions::get_option_map_in_node(string option_name, string map_key) {

    auto sequence_node = get_option_node(option_name);

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


int GOptions::getVerbosityFor(string tag) {
    auto verbosity_node = get_option_node("verbosity");

    for (auto v: verbosity_node) {
        if (v.begin()->first.as<string>() == tag) {
            return v.begin()->second.as<int>();
        }
    }

    return 0;
}


// print only the non default settings set by users
void GOptions::print_help() {

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
        option.print_help(false);
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
void GOptions::print_web_help() {

    exit(EXIT_SUCCESS);
}


// print options and switches values
void GOptions::save_options() {

    for (auto &s: switches) {
        string status = "false";
        if (s.second.getStatus()) status = "true";
        *yaml_conf << s.first + ": " + status << "," << endl;
    }

    for (auto &option: goptions) {
        option.save_option(yaml_conf);
    }

    yaml_conf->close();
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
GOptions &operator+=(GOptions &gopts, GOptions goptions_to_add) {
    gopts.addGOptions(goptions_to_add);
    return gopts;
}
