// goptions 
#include "goptions.h"
#include "goptionsConventions.h"
#include "gversion.h"

// gemc
#include "gutilities.h"

// c++
#include <iostream>

using namespace std;


using namespace gutilities;
//using namespace std;

// constructor:
// - load user defined options, add goptions options
// - assign internal options (gdebug, gstrict)
// - parse the base jcard plus all imported jcards
// - parse the command line options
// - get our own option
GOptions::GOptions(int argc, char *argv[], GOptions user_defined_options) {

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--h") == 0 || strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help();
        } else if (strcmp(argv[i], "-hweb") == 0) {
            print_web_help();
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--v") == 0 || strcmp(argv[i], "-version") == 0 || strcmp(argv[i], "--version") == 0) {
            print_version();
            exit(EXIT_SUCCESS);
        }
    }
    cout << endl;

    goptions += user_defined_options.goptions;
    switches += user_defined_options.switches;


    // goptions for all programs:
    addSwitch("gui", "use Graphical User Interface"); // gui mode


    // parsing command line to check if any switch is turned on
    for (int i = 1; i < argc; i++) {
        string candidateSwitch = string(argv[i]);
        if (candidateSwitch[0] == '-') {
            for (auto &[switchName, swiitchValue]: switches) {
                string candidateRoot = candidateSwitch.substr(1, candidateSwitch.size() - 1);

                if (switchName == candidateRoot) {
                    swiitchValue.turnOn();
                }
            }
        }
    }

    // finds the yamls
    vector <string> yaml_files = find_yaml(argc, argv);


    // parse the yaml files

    // parse command lines



}


// Finds the (first) configuration file (yaml).
vector <string> GOptions::find_yaml(int argc, char *argv[]) {
    vector <string> yaml_files;

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];

        size_t pos = arg.find(".yaml");
        if (pos != string::npos) yaml_files.push_back(arg);
    }

    if (yaml_files.size() == 0) {
        return {UNINITIALIZEDSTRINGQUANTITY};
    }

    return yaml_files;

}

// add a command line switch to the map of switches
void GOptions::addSwitch(string name, string description) {
    if (switches.find(name) == switches.end()) {
        switches[name] = GSwitch(description);
    } else {
        std::cerr << FATALERRORL << "the " << YELLOWHHL << name << RSTHHR << " switch is already present." << std::endl;
        gexit(EC__SWITCHALREADYPRESENT);
    }
}

// add a simple option to the map of options
void  GOptions::addOption(string name, string description, string defaultValue, string help) {

    // first, check that an option with the same name is not already defined
    if (goptions.size() == 0) {
        goptions.push_back(GOption(name, description, defaultValue, help));
    } else {
        for (auto &goption: goptions) {
            if (goption.name == name) {
                std::cerr << FATALERRORL << "the " << YELLOWHHL << name << RSTHHR << " option is already present." << std::endl;
                gexit(EC__NONCUMULATIVEALREADYPRESENT);
            } else {
                goptions.push_back(GOption(name, description, defaultValue, help));
            }
        }
    }
}


// add a map option to the map of options
void GOptions::addOption(string name, string description, vector <string> defaultValue, string help) {

//    // first, check that an option with the same name is not already defined
//    for (auto &goption: goptions) {
//        if (goption.name == name) {
//            std::cerr << FATALERRORL << "the " << YELLOWHHL << name << RSTHHR << " option is already present." << std::endl;
//            gexit(EC__NONCUMULATIVEALREADYPRESENT);
//        } else {
//            goptions.push_back(GOption(name, description, defaultValue, help));
//        }
//    }
}


// find GOption index from the vector<GOption>
// error if GOption is not found
//long GOptions::findOptionIndex(string name) {
//
//    for (auto it = goptions.begin(); it != goptions.end(); it++) {
//        if (it->name == name) {
//            return distance(goptions.begin(), it);
//        }
//    }
//
//    // not found, error
//    cerr << FATALERRORL << "the option " << YELLOWHHL << name << RSTHHR << " is not known to this system. " << endl;
//    cerr << "Use option " << PRINTALLOPTIONS << " to print all availaible options " << endl;
//    gexit(EC__NOOPTIONFOUND);
//
//    return UNINITIALIZEDNUMBERQUANTITY;
//}





// print only the non default settings set by users
void GOptions::print_settings(bool withDefaults) {
//    // making sure at least one option has value
//    bool canPrint = false;
//
//    for (auto &jOption: goptions) {
//        if (jOption.jOptionAssignedValues.size()) {
//            canPrint = true;
//        }
//    }
//
//    // nothing to do.
//    if (!canPrint) {
//        cout << KGRN << " No settings defined. " << RST << endl;
//        return;
//    }
//
//    if (withDefaults) {
//        cout << endl << KGRN << " All Settings: " << RST << endl << endl;
//    } else {
//        cout << endl << KGRN << " Non Default User Settings: " << RST << endl << endl;
//    }
//    // switches
//    for (auto [name, switchValue]: switches) {
//        if (withDefaults) {
//            // print all switches
//            cout << KGRN << ARROWITEM << name << RST << ": " << (switchValue.getStatus() ? "on" : "off") << endl;
//        } else {
//            // only print the active switches
//            if (switchValue.getStatus()) {
//                cout << KGRN << ARROWITEM << name << RST << ": on" << endl;
//            }
//        }
//    }
//
//    for (auto &jOption: goptions) {
//
//        jOption.printOption(withDefaults);
//
//        // non structured option, the jOptionAssignedValues has only one object, the json size is 1
//        if (jOption.jOptionAssignedValues.size() == 1 && jOption.jOptionAssignedValues.front().size() == 1) {
//        } else {
//            cout << endl;
//        }
//    }
    cout << endl;
}




//int GOptions::getInt(string tag) {
//    json jn = getNonStructuredOptionSingleValue(tag); // will exit if not found
//    return jn[tag].get<int>();
//}
//
//float GOptions::getFloat(string tag) {
//    json jn = getNonStructuredOptionSingleValue(tag); // will exit if not found
//    return jn[tag].get<float>();
//}
//
//double GOptions::getDouble(string tag) {
//    json jn = getNonStructuredOptionSingleValue(tag); // will exit if not found
//    return jn[tag].get<double>();
//}
//
//string GOptions::getString(string tag) {
//    json jn = getNonStructuredOptionSingleValue(tag); // will exit if not found
//    return jn[tag].get<string>();
//}
//
//bool GOptions::getSwitch(string tag) {
//    if (switches.find(tag) != switches.end()) {
//        return switches[tag].getStatus();
//    } else {
//        cerr << FATALERRORL << "the " << YELLOWHHL << tag << RSTHHR << " switch is not known to this system. " << endl;
//        gexit(EC__NOOPTIONFOUND);
//    }
//    return false;
//}





// overloaded operator to add option vectors
vector <GOption> &operator+=(vector <GOption> &original, vector <GOption> optionsToAdd) {

    for (const auto &optionToadd: optionsToAdd) {
        original.push_back(optionToadd);
    }

    return original;
}


map <string, GSwitch> &operator+=(map <string, GSwitch> &original, map <string, GSwitch> optionsToAdd) {

    for (const auto &[name, switchToAdd]: optionsToAdd) {
        original[name] = switchToAdd;
    }

    return original;
}

// #include <iomanip>

// print only the non default settings set by users
void GOptions::print_help() {

    long int helpSize = string(HELPFILLSPACE).size() + 1;
    cout.fill('.');

    cout << KGRN << " Usage: " << RST << endl << endl;

    for (auto &s: switches) {

        string help = "-" + s.first + RST + " ";
        cout << KGRN << ARROWITEM;
        cout << left;
        cout.width(helpSize);
        cout << help;
        cout << s.second.getDescription() << endl;
    }

    for (auto &option: goptions) {
        option.print_help();
    }

    cout << RST << endl;
    exit(EXIT_SUCCESS);
}


// print only the non default settings set by users
void GOptions::print_web_help() {

    long int helpSize = string(HELPFILLSPACE).size() + 1;
    cout.fill('.');

    cout << KRED << " Usage: " << RST << endl << endl;

//    for (auto &s: switches) {
//
//        string help = "-" + s.first + RST + " ";
//        cout << KGRN << ARROWITEM;
//        cout << left;
//        cout.width(helpSize);
//        cout << help;
//        cout << s.second.getDescription() << endl;
//    }
//
//    for (auto &jOption: goptions) {
//        jOption.print_option_help();
//        //	cout << endl;
//    }
//
//    cout << RST << endl;
    exit(EXIT_SUCCESS);
}

// introspection
void GOptions::print_version() {
    string asterisks = "**************************************************************";
    cout << endl << asterisks << endl;
    cout << " Gemc version: " << KGRN << gversion << RST << endl;
    cout << " Released on: " << KGRN << grelease_date << RST << endl;
    cout << " Reference: " << KGRN << greference << RST << endl;
    cout << " Homepage: " << KGRN << gweb << RST << endl;
    cout << " Author: " << KGRN << gauthor << RST << endl << endl;
    cout << asterisks << endl << endl;
}
