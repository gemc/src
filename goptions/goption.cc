// goptions
#include "goption.h"
#include "goptionsConventions.h"

// gemc
#include "gutilities.h"

// c++
#include <iostream>

using namespace std;

/**
 * @brief Sets the value of the scalar option.
 *
 * This function replaces any commas in the input string with empty spaces
 * and then assigns the modified string as the value for the scalar option.
 *
 * @param v The input string to be set as the value.
 */
 void GOption::set_scalar_value(const string &v) {

    if (v.empty()) return;  ///< Return early if the input string is empty.

    string value_to_set = gutilities::replaceCharInStringWithChars(v, ",", "");

    string key = value.begin()->first.as<string>();
    value[key] = value_to_set;
}


/**
 * @brief Sets the value of the option based on the parsed YAML node.
 *
 * This function handles both cumulative and non-cumulative options.
 * It checks for missing mandatory values and updates the option value accordingly.
 *
 * @param v The YAML node containing the new values for the option.
 */void GOption::set_value(const YAML::Node &v) {

    // if the option is cumulative,
    if (isCumulative) {

        // Sequence of maps: checks for missing mandatory values
        for (const auto &element: v) {
            if (!does_the_option_set_all_necessary_values(element)) {
                cerr << FATALERRORL << "Trying to set " << YELLOWHHL << name << RSTHHR << " but missing mandatory values." << endl;
                cerr << "        Use the option: " << YELLOWHHL << " help " << name << " " << RSTHHR << " for details." << endl << endl;
                exit(EC__MANDATORY_NOT_FILLED);
            }
        }

        value[name] = v;

        // Copy keys from the defaultValue map to the value map if they do not exist
        auto default_value_node = defaultValue.begin()->second;

        for (const auto &map_element_in_default_value: default_value_node) {
            for (YAML::const_iterator default_value_iterator = map_element_in_default_value.begin();
                 default_value_iterator != map_element_in_default_value.end(); ++default_value_iterator) {

                string default_key = default_value_iterator->first.as<string>();
                auto default_value = default_value_iterator->second;

                for (auto map_element_in_value: value[name]) {
                    bool key_found = false;
                    // checking if the key is already in the value
                    for (YAML::const_iterator value_iterator = map_element_in_value.begin();
                         value_iterator != map_element_in_value.end(); ++value_iterator) {
                        string value_key = value_iterator->first.as<string>();

                        if (default_key == value_key) {
                            key_found = true;
                            break;
                        }
                    }
                    if (!key_found) {
                        map_element_in_value[default_key] = default_value;
                    }
                }
            }
        }

    } else {

        // Update non-cumulative option values
        for (const auto &map_element_in_desired_value: v) {

            for (YAML::const_iterator desired_value_iterator = map_element_in_desired_value.begin();
                 desired_value_iterator != map_element_in_desired_value.end(); ++desired_value_iterator) {

                for (auto existing_map: value[name]) {
                    for (YAML::const_iterator existing_map_iterator = existing_map.begin();
                         existing_map_iterator != existing_map.end(); ++existing_map_iterator) {

                        string first_key = existing_map_iterator->first.as<string>();
                        string second_key = desired_value_iterator->first.as<string>();

                        if (first_key == second_key) {
                            existing_map[existing_map_iterator->first] = desired_value_iterator->second;
                        }
                    }
                }
            }
        }
    }
}

/**
 * @brief Ensures that all necessary variables marked as `NODFLT` are set.
 *
 * This function checks if all mandatory keys are present in the provided YAML node.
 *
 * @param v The YAML node to check.
 * @return True if all necessary values are set, false otherwise.
 */
 bool GOption::does_the_option_set_all_necessary_values(YAML::Node v) {
    vector <string> this_keys;

    switch (v.Type()) {
        case YAML::NodeType::Map:
            for (const auto &it : v) {
                this_keys.push_back(it.first.as<string>());
            }
            break;
        default:
            break;
    }

    for (const auto &key : mandatory_keys) {
        if (find(this_keys.begin(), this_keys.end(), key) == this_keys.end()) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Saves the option value to a YAML configuration file.
 *
 * This function writes the current value of the option to the specified YAML configuration file.
 *
 * @param yamlConf Pointer to the output file stream.
 */
 void GOption::saveOption(std::ofstream *yamlConf) const {

    // setting style to block
    // this does not work with command line passed values
    YAML::Node mutableValue = value;
    mutableValue.SetStyle(YAML::EmitterStyle::Block);  ///< Apply block style to the mutable copy.

    *yamlConf << mutableValue << std::endl;

}


/**
 * @brief Prints the help information for the option.
 *
 * This function outputs the help information to the console, either in detailed or summary form.
 *
 * @param detailed If true, prints detailed help; otherwise, prints a summary.
 */
 void GOption::printHelp(bool detailed) const {

    if (name == GVERSION_STRING) return;

    long int fill_width = string(HELPFILLSPACE).size() + 1;
    cout.fill('.');

    string helpString = "-" + name + RST;

    bool is_sequence = defaultValue.begin()->second.IsSequence();

    if (is_sequence) {
        helpString += "=<sequence>";
    } else {
        helpString += "=<value>";
    }
    helpString += " ";

    cout << KGRN << " " << left;
    cout.width(fill_width);

    if (detailed) {
        cout << helpString << ": " << description << endl;
        cout << endl;
        cout << detailedHelp() << endl;
    } else {
        cout << helpString << ": " << description << endl;
    }
}

/**
 * @brief Provides detailed help information for the option.
 *
 * This function returns a string containing detailed help information, including default values and descriptions.
 *
 * @return A string containing the detailed help information.
 */

string GOption::detailedHelp() const {
    string newHelp = "";

    YAML::Node yvalues = defaultValue.begin()->second;
    if (yvalues.IsSequence()) {
        newHelp += "\n";

        for (unsigned i = 0; i < (unsigned) yvalues.size(); i++) {
            YAML::Node this_node = yvalues[i];

            for (YAML::const_iterator it = this_node.begin(); it != this_node.end(); ++it) {
                cout << TGREENPOINTITEM << " " << KGRN << it->first.as<string>() << RST
                     << ": " << gvar_descs[i] << ". Default value: " << it->second.as<string>() << endl;
            }
        }
    }
    newHelp += "\n";

    vector <string> help_lines = gutilities::getStringVectorFromStringWithDelimiter(help, "\n");

    for (auto line: help_lines) {
        newHelp += GTAB + line + "\n";
    }
    return newHelp;

}
