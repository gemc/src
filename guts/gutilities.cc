// gutilities
#include "gutilities.h"
#include "gutsConventions.h"

// Numbers / strings with units / io interface to CLHEP units
#include "CLHEP/Units/PhysicalConstants.h"

// c++
// algorithm for 'transform'
#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <vector>
#include <charconv>
#include <filesystem>

namespace gutilities {
/*
 * Trim leading/trailing spaces and tabs from an owning std::string.
 *
 * Notes:
 * - Whitespace considered here is strictly ' ' and '\t' (tab).
 * - If the input is all whitespace (or empty), returns an empty string.
 *
 * See the API documentation in gutilities.h for full Doxygen docs.
 */
string removeLeadingAndTrailingSpacesFromString(const std::string& input) {
	size_t startPos = input.find_first_not_of(" \t"); // Find the first non-whitespace character
	size_t endPos   = input.find_last_not_of(" \t");  // Find the last non-whitespace character

	// If all spaces or empty, return an empty string
	if (startPos == std::string::npos || endPos == std::string::npos) { return ""; }

	// Return the substring between startPos and endPos
	return input.substr(startPos, endPos - startPos + 1);
}

/*
 * Fast trim for std::string_view.
 *
 * Notes:
 * - No allocations: adjusts the view by removing prefix/suffix.
 * - Uses std::isspace (locale-sensitive) to classify whitespace.
 *
 * See the API documentation in gutilities.h for full Doxygen docs.
 */
std::string_view removeLeadingAndTrailingSpacesFromString(std::string_view s) {
	while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.remove_prefix(1);
	while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.remove_suffix(1);
	return s;
}

/*
 * Remove all literal spaces ' ' from a string.
 *
 * See the API documentation in gutilities.h for full Doxygen docs.
 */
string removeAllSpacesFromString(const std::string& str) {
	string result = str;
	result.erase(std::remove(result.begin(), result.end(), ' '), result.end());
	return result;
}

/*
 * Extract the filename component from a POSIX-style path (splitting on '/').
 *
 * See the API documentation in gutilities.h for full Doxygen docs.
 */
string getFileFromPath(const std::string& path) {
	std::size_t lastSlashPos = path.find_last_of('/');
	if (lastSlashPos == std::string::npos) {
		// No slashes found, return the entire path
		return path;
	}
	return path.substr(lastSlashPos + 1);
}

/*
 * Extract the directory component from a POSIX-style path (splitting on '/').
 *
 * See the API documentation in gutilities.h for full Doxygen docs.
 */
string getDirFromPath(const std::string& path) {
	auto lastSlash = path.find_last_of('/');
	if (lastSlash == std::string::npos) return ".";
	return path.substr(0, lastSlash);
}

namespace fs = std::filesystem;


/*
 * Tokenize a string on whitespace into a vector.
 *
 * See the API documentation in gutilities.h for full Doxygen docs.
 */
vector<std::string> getStringVectorFromString(const std::string& input) {
	std::vector<std::string> pvalues;
	std::stringstream        plist(input);
	string                   tmp;
	while (plist >> tmp) {
		string trimmed = removeLeadingAndTrailingSpacesFromString(tmp);
		if (!trimmed.empty()) { pvalues.push_back(trimmed); }
	}
	return pvalues;
}

/*
 * Replace any character found in 'toReplace' with the string 'replacement'.
 *
 * See the API documentation in gutilities.h for full Doxygen docs.
 */
string replaceCharInStringWithChars(const std::string& input, const std::string& toReplace,
                                    const std::string& replacement) {
	string output;
	for (const char& ch : input) {
		if (toReplace.find(ch) != std::string::npos) { output.append(replacement); }
		else { output.push_back(ch); }
	}
	return output;
}

/*
 * Replace all occurrences of substring 'from' with substring 'to'.
 *
 * See the API documentation in gutilities.h for full Doxygen docs.
 */
string replaceAllStringsWithString(const string& source, const string& from, const string& to) {
	if (from.empty()) return source; // Avoid infinite loop

	string newString;
	size_t lastPos = 0;
	size_t findPos = source.find(from, lastPos);

	while (findPos != string::npos) {
		// Append part before the match and the replacement string
		newString.append(source, lastPos, findPos - lastPos);
		newString += to;
		lastPos   = findPos + from.length();
		findPos   = source.find(from, lastPos);
	}

	// Append the remaining part of the string after the last occurrence
	newString += source.substr(lastPos);

	return newString;
}


/*
 * Left-pad a string using the first character of 'c' until length reaches ndigits.
 *
 * See the API documentation in gutilities.h for full Doxygen docs.
 */
string fillDigits(const string& word, const string& c, int ndigits) {
	if (c.empty() || ndigits <= static_cast<int>(word.size())) return word; // Return original if no padding needed

	string filled;

	int toFill = ndigits - static_cast<int>(word.size());
	filled.reserve(ndigits);

	filled.append(toFill, c[0]); // Use the first character of the string 'c'
	filled += word;

	return filled;
}

// add near your includes:
#include <locale.h>   // strtod_l / _strtod_l

/**
 * @brief Parse a whole string_view as a double using the "C" numeric locale.
 *
 * This is a private helper (translation-unit local). It ensures locale-independent numeric parsing.
 * Parsing succeeds only if the entire string is consumed (no trailing garbage).
 *
 * @param sv String view containing the numeric text.
 * @param out Parsed value on success.
 * @return true on full-consume success; false otherwise.
 *
 * @note Private helper function: refer to it as \c parse_double_clocale (no \ref).
 */
static bool parse_double_clocale(std::string_view sv, double& out) {
	std::string tmp(sv); // strtod_l needs a 0-terminated buffer
#if defined(_WIN32)
	_locale_t loc = _create_locale(LC_NUMERIC, "C");
	char*     end = nullptr;
	out           = _strtod_l(tmp.c_str(), &end, loc);
	_free_locale(loc);
	return end == tmp.c_str() + tmp.size();
#else
	locale_t loc = newlocale(LC_NUMERIC_MASK, "C", (locale_t)0);
	char*    end = nullptr;
	out          = strtod_l(tmp.c_str(), &end, loc);
	freelocale(loc);
	return end == tmp.c_str() + tmp.size();
#endif
}


// --- strict, locale-independent getG4Number: only accepts '*' as unit sep ---
double getG4Number(const string& v, bool warnIfNotUnit) {
	string value = removeLeadingAndTrailingSpacesFromString(v);
	if (value.empty()) {
		std::cerr << FATALERRORL << "empty numeric string.\n";
		exit(EC__G4NUMBERERROR);
	}

	// Normalize a single decimal comma to dot when no dot is present
	if (value.find('.') == string::npos) {
		size_t firstComma = value.find(',');
		if (firstComma != string::npos && value.find(',', firstComma + 1) == string::npos) {
			value = replaceAllStringsWithString(value, ",", ".");
		}
	}

	const size_t starCount = static_cast<size_t>(std::count(value.begin(), value.end(), '*'));

	// --- Case 1: no '*' → pure number (strictly no trailing garbage) ---
	if (value.find('*') == string::npos) {
		double out = 0.0;
		// normalize a single decimal comma to dot if needed
		if (value.find('.') == string::npos) {
			auto firstComma = value.find(',');
			if (firstComma != string::npos && value.find(',', firstComma + 1) == string::npos)
				value = replaceAllStringsWithString(value, ",", ".");
		}
		if (!parse_double_clocale(value, out)) {
			std::cerr << FATALERRORL << "missing '*' before unit or invalid number in <" << v << ">.\n";
			exit(EC__G4NUMBERERROR);
		}
		if (warnIfNotUnit && out != 0.0) {
			std::cerr << " ! Warning: value " << v << " does not contain units." << std::endl;
		}
		return out;
	}


	// --- Case 2: must be exactly one '*' ---
	if (starCount > 1) {
		std::cerr << FATALERRORL << "multiple '*' separators are not allowed in <" << v << ">.\n";
		exit(EC__G4NUMBERERROR);
	}

	// --- Exactly one '*' → split "<number>*<unit>" ---
	const size_t pos   = value.find('*');
	string       left  = removeLeadingAndTrailingSpacesFromString(value.substr(0, pos));
	string       right = removeLeadingAndTrailingSpacesFromString(value.substr(pos + 1));
	if (left.empty() || right.empty()) {
		std::cerr << FATALERRORL << "expected '<number>*<unit>', got <" << v << ">.\n";
		exit(EC__G4NUMBERERROR);
	}

	// normalize a single decimal comma in the numeric part
	if (left.find('.') == string::npos) {
		auto c = left.find(',');
		if (c != string::npos && left.find(',', c + 1) == string::npos)
			left = replaceAllStringsWithString(left, ",", ".");
	}

	double numeric = 0.0;
	if (!parse_double_clocale(left, numeric)) {
		std::cerr << FATALERRORL << "invalid numeric part before '*' in <" << v << ">.\n";
		exit(EC__G4NUMBERERROR);
	}

	// sanitize unit and proceed with your existing unit table logic...
	right       = replaceAllStringsWithString(right, "µ", "u");
	string unit = convertToLowercase(right);

	// (keep your unitConversion map and SI prefix handling as-is)


	// Unit table (lowercase keys)
	static const std::unordered_map<string, double> unitConversion = {
		// length
		{"m", CLHEP::m}, {"cm", CLHEP::cm}, {"mm", CLHEP::mm},
		{"um", 1E-6 * CLHEP::m}, {"fm", 1E-15 * CLHEP::m},
		{"inch", 2.54 * CLHEP::cm}, {"inches", 2.54 * CLHEP::cm},
		// angle
		{"deg", CLHEP::deg}, {"degrees", CLHEP::deg}, {"arcmin", CLHEP::deg / 60.0},
		{"rad", CLHEP::rad}, {"mrad", CLHEP::mrad},
		// energy
		{"ev", CLHEP::eV}, {"kev", 1e3 * CLHEP::eV}, {"mev", CLHEP::MeV}, {"gev", CLHEP::GeV},
		// magnetic field
		{"t", CLHEP::tesla}, {"tesla", CLHEP::tesla}, {"t/m", CLHEP::tesla / CLHEP::m},
		{"gauss", CLHEP::gauss}, {"kilogauss", 1000.0 * CLHEP::gauss},
		// time
		{"s", CLHEP::s}, {"ns", CLHEP::ns}, {"ms", CLHEP::ms}, {"us", CLHEP::us},
		// dimensionless
		{"counts", 1.0}
	};

	// Exact unit match
	if (auto it = unitConversion.find(unit); it != unitConversion.end()) {
		return numeric * it->second;
	}

	// SI prefix handling: mT, uT, mm, um, etc.
	auto si_prefix_factor = [](char p) -> double {
		switch (p) {
		case 'Y': return 1e24;
		case 'Z': return 1e21;
		case 'E': return 1e18;
		case 'P': return 1e15;
		case 'T': return 1e12;
		case 'G': return 1e9;
		case 'M': return 1e6;
		case 'k': return 1e3;
		case 'h': return 1e2;
		case 'd': return 1e-1;
		case 'c': return 1e-2;
		case 'm': return 1e-3;
		case 'u': return 1e-6;
		case 'n': return 1e-9;
		case 'p': return 1e-12;
		case 'f': return 1e-15;
		case 'a': return 1e-18;
		case 'z': return 1e-21;
		case 'y': return 1e-24;
		default: return 0.0;
		}
	};

	if (unit.size() >= 2) {
		const double pf = si_prefix_factor(unit.front());
		if (pf != 0.0) {
			const string base = unit.substr(1);
			if (auto it2 = unitConversion.find(base); it2 != unitConversion.end()) {
				return numeric * pf * it2->second;
			}
		}
	}

	// Unknown unit: warn & return numeric part (keep your legacy behavior)
	std::cerr << GWARNING << ">" << right << "<: unit not recognized for string <" << v << ">" << std::endl;
	return numeric;
}


double getG4Number(double input, const string& unit) {
	string gnumber = std::to_string(input) + "*" + unit;
	return getG4Number(gnumber, true);
}

vector<double> getG4NumbersFromStringVector(const vector<string>& vstring, bool warnIfNotUnit) {
	vector<double> output;
	output.reserve(vstring.size());

	for (const auto& s : vstring) { output.push_back(getG4Number(s, warnIfNotUnit)); }

	return output;
}

vector<double> getG4NumbersFromString(const string& vstring, bool warnIfNotUnit) {
	return getG4NumbersFromStringVector(getStringVectorFromStringWithDelimiter(vstring, ","), warnIfNotUnit);
}


string parseFileAndRemoveComments(const string& filename, const string& commentChars, int verbosity) {
	// Reading file
	std::ifstream in(filename);
	if (!in) {
		std::cerr << FATALERRORL << "can't open input file " << filename << ". Check your spelling. " << std::endl;
		exit(EC__FILENOTFOUND);
	}

	std::stringstream strStream;
	if (verbosity > 0) { std::cout << std::endl << CIRCLEITEM << " Loading string from " << filename << std::endl; }
	strStream << in.rdbuf(); // Read the file
	in.close();

	string parsedString = strStream.str();

	// Removing all occurrences of commentChars
	size_t nFPos;
	while ((nFPos = parsedString.find(commentChars)) != string::npos) {
		size_t firstNL  = parsedString.rfind('\n', nFPos);
		size_t secondNL = parsedString.find('\n', nFPos);
		parsedString.erase(firstNL, secondNL - firstNL);
	}

	return parsedString;
}

string retrieveStringBetweenChars(const string& input, const string& firstDelimiter, const string& secondDelimiter) {
	size_t firstpos  = input.find(firstDelimiter);
	size_t secondpos = input.find(secondDelimiter);

	if (firstpos == string::npos || secondpos == string::npos) { return ""; }
	return input.substr(firstpos + firstDelimiter.length(), secondpos - firstpos - firstDelimiter.length());
}

vector<string> getStringVectorFromStringWithDelimiter(const string& input, const string& x) {
	vector<string> pvalues;
	string         tmp;

	for (char ch : input) {
		if (ch != x[0]) { tmp += ch; }
		else {
			if (!tmp.empty()) {
				pvalues.push_back(removeLeadingAndTrailingSpacesFromString(tmp));
				tmp.clear();
			}
		}
	}

	if (!tmp.empty()) { pvalues.push_back(removeLeadingAndTrailingSpacesFromString(tmp)); }

	return pvalues;
}


// string search for a path with <name> from a possible list of absolute paths
// returns UNINITIALIZEDSTRINGQUANTITY if not found
// the filesystem solution does not work on linux systems.
// TODO: periodically try this?
//#include <filesystem>
//
//    string searchForDirInLocations(string dirName, vector <string> possibleLocations) {
//
//        for (auto trialLocation: possibleLocations) {
//            string possibleDir = trialLocation + "/" + dirName;
//            if (std::filesystem::exists(possibleDir)) {
//                return possibleDir;
//            }
//        }
//        return UNINITIALIZEDSTRINGQUANTITY;
//    }
//
//    vector <string> getListOfFilesInDirectory(string dirName, vector <string> extensions) {
//
//        vector <string> fileList;
//
//        for (const auto &entry: std::filesystem::directory_iterator(dirName)) {
//            for (auto &extension: extensions) {
//                if (entry.path().extension() == extension) {
//                    fileList.push_back(entry.path().filename());
//                }
//            }
//        }
//
//        return fileList;
//    }
// end of TODO

#include <dirent.h>
#include <sys/stat.h>

bool directoryExists(const std::string& path) {
	struct stat info{};
	if (stat(path.c_str(), &info) != 0) {
		return false; // Path does not exist
	}
	return (info.st_mode & S_IFDIR) != 0; // Check if it's a directory
}

string searchForDirInLocations(const string& dirName, const vector<string>& possibleLocations) {
	for (const auto& trialLocation : possibleLocations) {
		string possibleDir = trialLocation + "/" + dirName;
		if (directoryExists(possibleDir)) { return possibleDir; }
	}
	return "UNINITIALIZEDSTRINGQUANTITY";
}


bool hasExtension(const std::string& filename, const std::vector<std::string>& extensions) {
	for (const auto& ext : extensions) {
		if (filename.size() >= ext.size() &&
			filename.compare(filename.size() - ext.size(), ext.size(), ext) == 0) { return true; }
	}
	return false;
}

vector<string> getListOfFilesInDirectory(const string& dirName, const vector<string>& extensions) {
	vector<string> fileList;

	DIR* dir = opendir(dirName.c_str());
	if (dir) {
		struct dirent* entry;
		while ((entry = readdir(dir)) != nullptr) {
			struct stat info{};
			string      filepath = dirName + "/" + entry->d_name;
			if (stat(filepath.c_str(), &info) == 0 && S_ISREG(info.st_mode)) {
				string filename = entry->d_name;
				if (hasExtension(filename, extensions)) { fileList.push_back(filename); }
			}
		}
		closedir(dir);
	}

	return fileList;
}

string convertToLowercase(const string& str) {
	string lower = str;
	transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
	return lower;
}


template <class KEY, class VALUE>
vector<KEY> getKeys(const map<KEY, VALUE>& map) {
	vector<KEY> keys;
	keys.reserve(map.size()); // Reserve space for efficiency

	for (const auto& it : map) { keys.push_back(it.first); }

	return keys;
}

randomModel stringToRandomModel(const std::string& str) {
	static const std::unordered_map<std::string, randomModel> strToEnum = {
		{"uniform", uniform},
		{"gaussian", gaussian},
		{"cosine", cosine},
		{"sphere", sphere}
	};

	auto it = strToEnum.find(str);
	if (it != strToEnum.end()) { return it->second; }
	else { throw std::invalid_argument("Invalid string for randomModel: " + str); }
}


G4Colour makeG4Colour(std::string_view code, double opacity) {
	if (code.empty()) throw std::invalid_argument("empty colour string");
	if (code.front() == '#') code.remove_prefix(1);
	if (code.size() != 6)
		throw std::invalid_argument("colour must have 6 or 7 hex digits");

	auto hexNibble = [](char c) -> unsigned {
		if ('0' <= c && c <= '9') return c - '0';
		c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
		if ('A' <= c && c <= 'F') return c - 'A' + 10;
		throw std::invalid_argument("invalid hex digit");
	};

	// ---- parse RRGGBB ----
	unsigned rgb = 0;
	for (int i = 0; i < 6; ++i)
		rgb = (rgb << 4) | hexNibble(code[i]);

	auto   byteToDouble = [](unsigned byte) { return byte / 255.0; };
	double r            = byteToDouble((rgb >> 16) & 0xFF);
	double g            = byteToDouble((rgb >> 8) & 0xFF);
	double b            = byteToDouble(rgb & 0xFF);

	return {r, g, b, opacity}; // G4Colour
}

std::optional<std::string> searchForFileInLocations(
	const std::vector<std::string>& locations,
	std::string_view                filename) {
	namespace fs = std::filesystem;

	for (const auto& loc : locations) {
		if (loc.empty()) continue;

		fs::path p(loc);
		fs::path candidate = (!filename.empty() && fs::is_directory(p))
			                     ? (p / filename)
			                     : p;

		std::error_code ec;
		const bool      ok = fs::exists(candidate, ec) && fs::is_regular_file(candidate, ec);
		if (ok) return candidate.string();
	}
	return std::nullopt;
}

bool is_unset(std::string_view s) {
	s = removeLeadingAndTrailingSpacesFromString(s);
	if (s.empty()) return true;
	// match your sentinel and YAML nully spellings
	auto eq = [](std::string_view a, std::string_view b) {
		if (a.size() != b.size()) return false;
		for (size_t i = 0; i < a.size(); ++i)
			if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i])))
				return false;
		return true;
	};
	return eq(s, UNINITIALIZEDSTRINGQUANTITY) || eq(s, "null") || eq(s, "~");
}

void apply_uimanager_commands(const std::string& command) {
	G4UImanager* g4uim = G4UImanager::GetUIpointer();
	if (g4uim == nullptr) { return; }
	g4uim->ApplyCommand(command);
}
}
