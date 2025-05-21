#include "goption.h"
#include "goptionsConventions.h"
#include "gutilities.h"

// gemc
#include "gutsConventions.h"

#include <iostream>
#include <algorithm>
#include <functional>

using std::cerr;
using std::endl;
using std::cout;
using std::left;
using std::string;
using std::vector;

/**
 * @brief Sets the value of the scalar option.
 *
 * This function replaces any commas in the input string with empty spaces and assigns
 * the modified string as the option’s value.
 *
 * @param v The input string to be set as the value.
 */
void GOption::set_scalar_value(const string& v) {
	if (v.empty()) return;
	string value_to_set = gutilities::replaceCharInStringWithChars(v, ",", "");
	auto   key          = value.begin()->first.as<string>();
	value[key]          = value_to_set;
}

/**
 * @brief Sets the option’s value based on the provided YAML node.
 *
 * Handles both cumulative and non-cumulative options. For cumulative options, the function
 * checks for missing mandatory keys and merges default values.
 *
 * @param v The YAML node containing the new value(s) for the option.
 */
void GOption::set_value(const YAML::Node& v) {
	if (isCumulative) {
		for (const auto& element : v) {
			if (!does_the_option_set_all_necessary_values(element)) {
				cerr << FATALERRORL << "Trying to set " << YELLOWHHL << name << RSTHHR
					<< " but missing mandatory values." << endl;
				cerr << "        Use the option: " << YELLOWHHL << " help " << name
					<< " " << RSTHHR << " for details." << endl << endl;
				exit(EC__MANDATORY_NOT_FILLED);
			}
		}
		value[name]             = v;
		auto default_value_node = defaultValue.begin()->second;
		for (const auto& map_element_in_default_value : default_value_node) {
			for (auto default_value_iterator = map_element_in_default_value.begin();
			     default_value_iterator != map_element_in_default_value.end(); ++default_value_iterator) {
				auto default_key   = default_value_iterator->first.as<string>();
				auto default_value = default_value_iterator->second;
				for (auto map_element_in_value : value[name]) {
					bool key_found = false;
					for (auto value_iterator = map_element_in_value.begin();
					     value_iterator != map_element_in_value.end(); ++value_iterator) {
						auto value_key = value_iterator->first.as<string>();
						if (default_key == value_key) {
							key_found = true;
							break;
						}
					}
					if (!key_found) { map_element_in_value[default_key] = default_value; }
				}
			}
		}
	}
	else {
		for (const auto& map_element_in_desired_value : v) {
			for (auto desired_value_iterator = map_element_in_desired_value.begin();
			     desired_value_iterator != map_element_in_desired_value.end(); ++desired_value_iterator) {
				for (auto existing_map : value[name]) {
					for (auto existing_map_iterator = existing_map.begin();
					     existing_map_iterator != existing_map.end(); ++existing_map_iterator) {
						auto first_key  = existing_map_iterator->first.as<string>();
						auto second_key = desired_value_iterator->first.as<string>();
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
 * @brief Checks if all mandatory keys (marked as NODFLT) are present in the YAML node.
 *
 * @param v The YAML node to check.
 * @return True if all mandatory keys are present; false otherwise.
 */
bool GOption::does_the_option_set_all_necessary_values(const YAML::Node& v) {
	vector<string> this_keys;
	if (v.Type() == YAML::NodeType::Map) { for (const auto& it : v) { this_keys.push_back(it.first.as<string>()); } }
	for (const auto& key : mandatory_keys) {
		if (find(this_keys.begin(), this_keys.end(), key) == this_keys.end()) { return false; }
	}
	return true;
}

/**
 * @brief Saves the current option value to a YAML configuration file.
 *
 * The function sets the YAML emitter style to block and writes the option value.
 *
 * @param yamlConf Pointer to the output file stream.
 */
void GOption::saveOption(std::ofstream* yamlConf) const
{
	std::vector<std::string> missing;   // paths of null values

	// --------------------------------------------------------------------
	// recursive lambda: returns a *new* node with nulls → "not provided"
	// --------------------------------------------------------------------
	std::function<YAML::Node(YAML::Node, std::string)> clean =
		[&](YAML::Node n, const std::string& path) -> YAML::Node
		{
			if (n.IsNull()) {
				missing.push_back(path.empty() ? name : path);
				return YAML::Node("not provided");          // null replaced
			}

			if (n.IsMap()) {
				YAML::Node res(YAML::NodeType::Map);
				for (auto it : n) {
					const std::string key = it.first.as<std::string>();
					res[it.first] = clean(it.second,
										  path.empty() ? key : path + "." + key);
				}
				return res;
			}

			if (n.IsSequence()) {
				YAML::Node res(YAML::NodeType::Sequence);
				for (std::size_t i = 0; i < n.size(); ++i)
					res.push_back(clean(n[i],
										path + "[" + std::to_string(i) + "]"));
				return res;
			}

			return n;  // scalar, already OK
		};

	YAML::Node out = clean(value, "");          // fully cleaned copy

	// --------------------------------------------------------------------
	// write one comment line per missing entry
	// --------------------------------------------------------------------
	for (const auto& p : missing)
		*yamlConf << "# " << p << " not provided\n";

	// write the YAML itself (block style)
	out.SetStyle(YAML::EmitterStyle::Block);
	*yamlConf << out << '\n';
}




/**
 * @brief Prints help information for the option.
 *
 * Depending on the 'detailed' flag, prints either a brief summary or detailed help
 * (including default values and descriptions).
 *
 * @param detailed If true, prints detailed help; otherwise, prints a summary.
 */
void GOption::printHelp(bool detailed) const {
	if (name == GVERSION_STRING) return;
	long int fill_width = string(HELPFILLSPACE).size() + 1;
	cout.fill('.');
	string helpString  = "-" + name + RST;
	bool   is_sequence = defaultValue.begin()->second.IsSequence();
	helpString += is_sequence ? "=<sequence>" : "=<value>";
	helpString += " ";
	cout << KGRN << " " << left;
	cout.width(fill_width);
	if (detailed) {
		cout << helpString << ": " << description << endl << endl;
		cout << detailedHelp() << endl;
	}
	else { cout << helpString << ": " << description << endl; }
}

/**
 * @brief Generates detailed help text for the option.
 *
 * This function returns a string containing detailed help information,
 * including default values and associated descriptions.
 *
 * @return A string with detailed help information.
 */
string GOption::detailedHelp() const {
	string     newHelp;
	YAML::Node yvalues = defaultValue.begin()->second;
	if (yvalues.IsSequence()) {
		newHelp += "\n";
		for (unsigned i = 0; i < yvalues.size(); i++) {
			YAML::Node this_node = yvalues[i];
			for (auto it = this_node.begin(); it != this_node.end(); ++it) {
				cout << TGREENPOINTITEM << " " << KGRN << it->first.as<string>() << RST
					<< ": " << gvar_descs[i] << ". Default value: " << it->second.as<string>() << endl;
			}
		}
	}
	newHelp += "\n";
	vector<string> help_lines = gutilities::getStringVectorFromStringWithDelimiter(help, "\n");
	for (const auto& line : help_lines) { newHelp += GTAB + line + "\n"; }
	return newHelp;
}

/**
 * @brief Sets the value of a sub–option using dot–notation.
 *
 * For options that are structured as maps or sequences of maps, this function updates a single
 * sub–option (e.g. for "-debug.general=true", it updates the "general" key within "debug").
 *
 * @param subkey The key within the option to update.
 * @param subvalue The new value for the sub–option as a string.
 */
void GOption::set_sub_option_value(const string& subkey, const string& subvalue) {
	YAML::Node option_node = value.begin()->second;
	if (option_node.IsSequence()) {
		bool updated = false;
		for (auto it = option_node.begin(); it != option_node.end(); ++it) {
			if ((*it).IsMap() && (*it)[subkey]) {
				(*it)[subkey] = YAML::Load(subvalue);
				updated       = true;
			}
		}
		if (!updated) {
			cerr << "Sub-option key '" << subkey << "' not found in option '" << name << "'." << endl;
			exit(EC__NOOPTIONFOUND);
		}
	}
	else if (option_node.IsMap()) {
		if (option_node[subkey]) { option_node[subkey] = YAML::Load(subvalue); }
		else {
			cerr << "Sub-option key '" << subkey << "' not found in option '" << name << "'." << endl;
			exit(EC__NOOPTIONFOUND);
		}
	}
	else {
		cerr << "Option '" << name << "' is not structured to accept sub–options." << endl;
		exit(EC__NOOPTIONFOUND);
	}
}
