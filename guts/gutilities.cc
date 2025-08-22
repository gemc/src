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

string removeLeadingAndTrailingSpacesFromString(const std::string& input) {
	size_t startPos = input.find_first_not_of(" \t"); // Find the first non-whitespace character
	size_t endPos   = input.find_last_not_of(" \t");  // Find the last non-whitespace character

	// If all spaces or empty, return an empty string
	if (startPos == std::string::npos || endPos == std::string::npos) { return ""; }

	// Return the substring between startPos and endPos
	return input.substr(startPos, endPos - startPos + 1);
}

string removeAllSpacesFromString(const std::string& str) {
	string result = str;
	result.erase(std::remove(result.begin(), result.end(), ' '), result.end());
	return result;
}

string getFileFromPath(const std::string& path) {
	std::size_t lastSlashPos = path.find_last_of('/');
	if (lastSlashPos == std::string::npos) {
		// No slashes found, return the entire path
		return path;
	}
	return path.substr(lastSlashPos + 1);
}

string getDirFromPath(const std::string& path) {
	auto lastSlash = path.find_last_of('/');
	if (lastSlash == std::string::npos) return ".";
	return path.substr(0, lastSlash);
}

namespace fs = std::filesystem;


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

// Replace all occurences of specific chars in a string with a string
string replaceCharInStringWithChars(const std::string& input, const std::string& toReplace,
                                    const std::string& replacement) {
	string output;
	for (const char& ch : input) {
		if (toReplace.find(ch) != std::string::npos) { output.append(replacement); }
		else { output.push_back(ch); }
	}
	return output;
}

string replaceAllStringsWithString(const string& source, const string& from, const string& to) {
	if (from.empty()) return source; // Avoid infinite loop

	string newString;
	size_t lastPos = 0;
	size_t findPos = source.find(from, lastPos);

	while (findPos != string::npos) {
		// Append part before the match and the replacement string
		newString.append(source, lastPos, findPos - lastPos);
		newString += to;
		lastPos = findPos + from.length();
		findPos = source.find(from, lastPos);
	}

	// Append the remaining part of the string after the last occurrence
	newString += source.substr(lastPos);

	return newString;
}


string fillDigits(const string& word, const string& c, int ndigits) {
	if (c.empty() || ndigits <= static_cast<int>(word.size())) return word; // Return original if no padding needed

	string filled;

	int toFill = ndigits - static_cast<int>(word.size());
	filled.reserve(ndigits);

	filled.append(toFill, c[0]); // Use the first character of the string 'c'
	filled += word;

	return filled;
}


double getG4Number(const string& v, bool warnIfNotUnit) {
	string value = removeLeadingAndTrailingSpacesFromString(v);

	// If no '*' is found, the input is assumed to be a number without units
	if (value.find('*') == string::npos) {
		if (!value.empty() && warnIfNotUnit && stod(value) != 0) { std::cerr << " ! Warning: value " << v << " does not contain units." << std::endl; }

		try { return stod(value); }
		catch (const std::exception& e) {
			std::cerr << FATALERRORL << "stod exception in gutilities: could not convert string to double. "
				<< "Value: >" << v << "<, error: " << e.what() << std::endl;
			exit(EC__G4NUMBERERROR);
		}
	}
	else {
		// Split the input string into the numeric part and the unit part
		size_t pos       = value.find('*');
		string rootValue = value.substr(0, pos);
		string units     = value.substr(pos + 1);

		double answer = 0;
		try { answer = stod(rootValue); }
		catch (const std::exception& e) {
			std::cerr << FATALERRORL << "stod exception in gutilities: could not convert string to double. "
				<< "Value: >" << v << "<, error: " << e.what() << std::endl;
			exit(EC__G4NUMBERERROR);
		}

		// Map of unit conversions for easier lookup and maintenance
		static const std::unordered_map<string, double> unitConversion = {
			{"m", CLHEP::m},
			{"cm", CLHEP::cm},
			{"mm", CLHEP::mm},
			{"um", 1E-6 * CLHEP::m},
			{"fm", 1E-15 * CLHEP::m},
			{"inches", 2.54 * CLHEP::cm},
			{"inch", 2.54 * CLHEP::cm},
			{"deg", CLHEP::deg},
			{"degrees", CLHEP::deg},
			{"arcmin", CLHEP::deg / 60.0},
			{"rad", CLHEP::rad},
			{"mrad", CLHEP::mrad},
			{"eV", CLHEP::eV},
			{"MeV", CLHEP::MeV},
			{"KeV", 0.001 * CLHEP::MeV},
			{"GeV", CLHEP::GeV},
			{"T", CLHEP::tesla},
			{"T/m", CLHEP::tesla / CLHEP::m},
			{"Tesla", CLHEP::tesla},
			{"gauss", CLHEP::gauss},
			{"kilogauss", CLHEP::gauss * 1000},
			{"s", CLHEP::s},
			{"ns", CLHEP::ns},
			{"ms", CLHEP::ms},
			{"us", CLHEP::us},
			{"counts", 1}
		};

		auto it = unitConversion.find(units);
		if (it != unitConversion.end()) { answer *= it->second; }
		else { std::cerr << GWARNING << ">" << units << "<: unit not recognized for string <" << v << ">" << std::endl; }

		return answer;
	}
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


G4Colour makeColour(std::string_view code) {
	if (code.empty()) throw std::invalid_argument("empty colour string");
	if (code.front() == '#') code.remove_prefix(1);
	if (code.size() != 6 && code.size() != 7)
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
		rgb    = (rgb << 4) | hexNibble(code[i]);

	auto   byteToDouble = [](unsigned byte) { return byte / 255.0; };
	double r            = byteToDouble((rgb >> 16) & 0xFF);
	double g            = byteToDouble((rgb >> 8) & 0xFF);
	double b            = byteToDouble(rgb & 0xFF);

	// ---- optional transparency nibble ----
	double a = 1.0;
	if (code.size() == 7)
		a = hexNibble(code[6]) / 15.0;

	return {r, g, b, a}; // G4Colour
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

}
