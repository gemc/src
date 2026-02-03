/**
 * @file goption.cc
 * @brief Implementation of \ref GOption : the option container used by \ref GOptions : .
 *
 * @details
 * \ref GOption : supports:
 * - **Scalar options** (single value) stored as a YAML scalar.
 * - **Structured options** stored as YAML maps or sequences of maps.
 *
 * Structured options can be **cumulative** (sequence of maps). In that mode:
 * - Some keys may be mandatory (schema value is \ref goptions::NODFLT : ).
 * - Missing non-mandatory keys can be filled from schema defaults.
 */

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

// See goption.h for API docs.
/*
 * Implementation notes:
 * - Scalar values are normalized to preserve legacy comma-delimited payloads.
 * - For structured options, the behavior differs between cumulative and non-cumulative schemas.
 */
void GOption::set_scalar_value(const string& v) {
	if (v.empty()) return;

	// Legacy normalization: remove commas so payloads like "a,b,c" remain shell-friendly.
	string value_to_set = gutilities::replaceCharInStringWithChars(v, ",", "");

	// Scalar options are stored as a single-entry map: { <name>: <scalar> }.
	auto key   = value.begin()->first.as<string>();
	value[key] = value_to_set;
}

// See goption.h for API docs.
/*
 * Implementation notes:
 * - Cumulative options store a user-provided sequence of maps and then back-fill optional keys
 *   from the schema defaults.
 * - Non-cumulative structured options update existing key/value pairs in-place (by matching keys).
 */
void GOption::set_value(const YAML::Node& v) {
	if (isCumulative) {
		// Validate that each user-provided entry includes all mandatory keys.
		for (const auto& element : v) {
			if (!does_the_option_set_all_necessary_values(element)) {
				cerr << FATALERRORL << "Trying to set " << YELLOWHHL << name << RSTHHR
					<< " but missing mandatory values." << endl;
				cerr << "        Use the option: " << YELLOWHHL << " help " << name
					<< " " << RSTHHR << " for details." << endl << endl;
				exit(EC__MANDATORY_NOT_FILLED);
			}
		}

		// Store the full sequence exactly as provided by the user.
		value[name] = v;

		// Back-fill optional keys from the schema default sequence.
		// The default schema is stored as a sequence of single-entry maps.
		auto default_value_node = defaultValue.begin()->second;

		for (const auto& map_element_in_default_value : default_value_node) {
			for (auto default_value_iterator = map_element_in_default_value.begin();
			     default_value_iterator != map_element_in_default_value.end(); ++default_value_iterator) {
				auto default_key   = default_value_iterator->first.as<string>();
				auto default_value = default_value_iterator->second;

				// For each user entry, ensure default_key exists; if not, assign schema default.
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

					if (!key_found) {
						map_element_in_value[default_key] = default_value;
					}
				}
			}
		}
	}
	else {
		// Non-cumulative structured update:
		// Iterate over desired values and update matching keys in the existing stored structure.
		for (const auto& map_element_in_desired_value : v) {
			for (auto desired_value_iterator = map_element_in_desired_value.begin();
			     desired_value_iterator != map_element_in_desired_value.end(); ++desired_value_iterator) {
				for (auto existing_map : value[name]) {
					for (auto existing_map_iterator = existing_map.begin();
					     existing_map_iterator != existing_map.end(); ++existing_map_iterator) {
						auto first_key  = existing_map_iterator->first.as<string>();
						auto second_key = desired_value_iterator->first.as<string>();

						// Only update entries whose key matches the requested update key.
						if (first_key == second_key) {
							existing_map[existing_map_iterator->first] = desired_value_iterator->second;
						}
					}
				}
			}
		}
	}
}

// See goption.h for API docs.
/*
 * Implementation notes:
 * - The input is expected to be one element of a cumulative sequence (typically a map).
 * - We only check keys; type/shape constraints are not enforced here.
 */
bool GOption::does_the_option_set_all_necessary_values(const YAML::Node& v) {
	vector<string> this_keys;
	if (v.Type() == YAML::NodeType::Map) {
		for (const auto& it : v) {
			this_keys.push_back(it.first.as<string>());
		}
	}

	for (const auto& key : mandatory_keys) {
		if (find(this_keys.begin(), this_keys.end(), key) == this_keys.end()) {
			return false;
		}
	}
	return true;
}

// See goption.h for API docs.
/*
 * Implementation notes:
 * - Writes comment lines for nulls so the saved YAML explains which values were not provided.
 * - Produces a cleaned copy of the node (nulls replaced) to keep output valid YAML scalars.
 */
void GOption::saveOption(std::ofstream* yamlConf) const {
	std::vector<std::string> missing; // paths of null values

	// --------------------------------------------------------------------
	// recursive lambda: returns a *new* node with nulls → "not provided"
	// --------------------------------------------------------------------
	std::function<YAML::Node(YAML::Node, std::string)> clean =
		[&](YAML::Node n, const std::string& path) -> YAML::Node {
		if (n.IsNull()) {
			missing.push_back(path.empty() ? name : path);
			return YAML::Node("not provided"); // null replaced
		}

		if (n.IsMap()) {
			YAML::Node res(YAML::NodeType::Map);
			for (auto it : n) {
				const std::string key = it.first.as<std::string>();
				res[it.first]         = clean(it.second, path.empty() ? key : path + "." + key);
			}
			return res;
		}

		if (n.IsSequence()) {
			YAML::Node res(YAML::NodeType::Sequence);
			for (std::size_t i = 0; i < n.size(); ++i) {
				res.push_back(clean(n[i], path + "[" + std::to_string(i) + "]"));
			}
			return res;
		}

		return n; // scalar, already OK
	};

	YAML::Node out = clean(value, ""); // fully cleaned copy

	// --------------------------------------------------------------------
	// write one comment line per missing entry
	// --------------------------------------------------------------------
	for (const auto& p : missing) {
		*yamlConf << "# " << p << " not provided\n";
	}

	// write the YAML itself (block style)
	out.SetStyle(YAML::EmitterStyle::Block);
	*yamlConf << out << '\n';
}


// See goption.h for API docs.
/*
 * Implementation notes:
 * - Summary help is a single aligned line.
 * - Detailed help includes schema defaults + extended multi-line help payload.
 */
void GOption::printHelp(bool detailed) const {
	if (name == GVERSION_STRING) return;

	long int fill_width = string(HELPFILLSPACE).size() + 1;
	cout.fill('.');

	string helpString  = "-" + name + RST;
	bool   is_sequence = defaultValue.begin()->second.IsSequence();
	helpString         += is_sequence ? "=<sequence>" : "=<value>";
	helpString         += " ";

	cout << KGRN << " " << left;
	cout.width(fill_width);

	if (detailed) {
		cout << helpString << ": " << description << endl << endl;
		cout << detailedHelp() << endl;
	}
	else {
		cout << helpString << ": " << description << endl;
	}
}

// See goption.h for API docs.
/*
 * Implementation notes:
 * - If the default schema is a sequence, print each key with its per-key description and default value.
 * - Then append the free-form help text, preserving user formatting.
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

	newHelp                   += "\n";
	vector<string> help_lines = gutilities::getStringVectorFromStringWithDelimiter(help, "\n");
	for (const auto& line : help_lines) {
		newHelp += GTAB + line + "\n";
	}

	return newHelp;
}

// See goption.h for API docs.
/*
 * Implementation notes:
 * - Dot-notation updates apply to existing structured storage.
 * - If the stored node is a sequence, all map elements containing subkey are updated.
 * - If the stored node is a map, only that entry is updated.
 */
void GOption::set_sub_option_value(const string& subkey, const string& subvalue) {
	YAML::Node option_node = value.begin()->second;

	if (option_node.IsSequence()) {
		bool updated = false;

		for (auto it = option_node.begin(); it != option_node.end(); ++it) {
			// Only update entries that are maps and already contain subkey.
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
		if (option_node[subkey]) {
			option_node[subkey] = YAML::Load(subvalue);
		}
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
