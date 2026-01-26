#pragma once

// c++
#include <vector>
#include <string>
#include <map>
#include <string_view>
#include <optional>

// geant4
#include "G4Colour.hh"
#include "G4UImanager.hh" // Geant4 UI manager access

namespace gutilities {
using std::string;
using std::vector;
using std::map;

/**
* @brief Removes leading and trailing spaces and tabs from a string.
*
* This function trims any whitespace, including spaces and tabs, from both the
* beginning and the end of the input string.
*
* @param input The string to be trimmed.
* @return A new string with leading and trailing spaces and tabs removed.
*/
std::string removeLeadingAndTrailingSpacesFromString(const std::string& input);

std::string_view removeLeadingAndTrailingSpacesFromString(const std::string_view input);

/**
* @brief Removes all spaces from a string.
*
* This function removes all spaces from the input string and returns the modified string.
*
* @param str The input string from which spaces will be removed.
* @return A new string with all spaces removed.
*/
std::string removeAllSpacesFromString(const std::string& str);

/**
* @brief Extracts the filename from a given file path.
*
*  This function extracts and returns the filename from a given POSIX file path.
*
* @param path The input file path.
* @return The filename extracted from the path.
*/
std::string getFileFromPath(const std::string& path);

/**
* @brief Extracts the directory path from a given file path.
*
* This function extracts and returns the directory path from a given POSIX file path.
*
* @param path The input file path.
* @return The directory path extracted from the path.
*/
std::string getDirFromPath(const std::string& path);

std::optional<std::string> searchForFileInLocations(
    const std::vector<std::string>& locations,
    std::string_view                filename);

/**
* @brief Splits a string into a vector of strings using spaces as delimiters.
*
* This function splits the input string into a vector of strings, using spaces as the delimiter,
* and removes leading and trailing spaces from each substring.
*
* @param input The input string to split.
* @return A vector of strings obtained by splitting the input string.
*/
std::vector<std::string> getStringVectorFromString(const std::string& input);

/**
* @brief Replaces all occurrences of specified characters in a string with another string.
*
* This function replaces every occurrence of any character from the 'toReplace' string
* in the input string with the specified replacement string.
*
* @param input The input string in which characters are to be replaced.
* @param toReplace A string containing characters to be replaced.
* @param replacement The string to replace each occurrence of characters in 'toReplace'.
* @return A new string with specified characters replaced by the replacement string.
*/
std::string replaceCharInStringWithChars(const std::string& input, const std::string& toReplace,
                                         const std::string& replacement);


/**
* @brief Replaces all occurrences of a substring with another string.
*
* This function searches for all occurrences of the specified substring in the source string
* and replaces them with the provided replacement string.
*
* @param source The original string where replacements will occur.
* @param from The substring to be replaced.
* @param to The string to replace each occurrence of the substring.
* @return A new string with all occurrences of the substring replaced.
*/
string replaceAllStringsWithString(const string& source, const string& from, const string& to);

/**
* @brief Pads a string with a specified character until it reaches a desired length.
*
* This function prepends the specified character to the given word until its length
* is equal to the specified number of digits.
*
* @param word The original string to be padded.
* @param c The character to use for padding.
* @param ndigits The target length of the padded string.
* @return A new string padded to the specified length.
*/
string fillDigits(const string& word, const string& c, int ndigits);

/**
* @brief Converts a string representation of a number with optional units to a double.
*
* This function extracts the numeric value from the input string and multiplies it by the appropriate unit conversion factor.
* If no units are specified, it issues a warning if `warnIfNotUnit` is true and the value is non-zero.
*
* @param v The input string containing the number and optional units, formatted as "number*unit".
* @param warnIfNotUnit Flag indicating whether to issue a warning if the units are not present.
* @return The numeric value converted to a double, taking into account the units if provided.
*/
double getG4Number(const string& v, bool warnIfNotUnit = false);

/**
* @brief Converts a numeric value with a unit into a G4 number format and returns the converted value.
*
* This function concatenates a numeric value and a unit into a G4 number format and then converts it to the appropriate unit.
*
* @param input The numeric value to convert.
* @param unit The unit associated with the numeric value.
* @return The converted numeric value as a double.
*/
double getG4Number(double input, const string& unit);

/**
* @brief Converts a vector of strings representing numbers with units to a vector of doubles.
*
* This function parses each string in the input vector and converts it into a double, taking into account the specified units.
*
* @param vstring A vector of strings, each containing a number with optional units.
* @param warnIfNotUnit Flag indicating whether to issue a warning if the units are not present.
* @return A vector of doubles representing the converted numeric values.
*/
vector<double> getG4NumbersFromStringVector(const vector<string>& vstring, bool warnIfNotUnit = false);

/**
* @brief Converts a comma-separated string of numbers with units to a vector of doubles.
*
* This function splits the input string by commas and converts each segment to a double, taking into account the specified units.
*
* @param vstring A comma-separated string of numbers with optional units.
* @param warnIfNotUnit Flag indicating whether to issue a warning if the units are not present.
* @return A vector of doubles representing the converted numeric values.
*/
vector<double> getG4NumbersFromString(const string& vstring, bool warnIfNotUnit = false);

/**
* @brief Parses a file and removes all lines containing specified comment characters.
*
* This function reads the content of a file, removes lines containing the specified comment characters, and returns the processed string.
*
* @param filename The name of the file to parse.
* @param commentChars The characters indicating the start of a comment in the file.
* @param verbosity The verbosity level for logging information.
* @return A string representing the content of the file with comments removed.
*/
string parseFileAndRemoveComments(const string& filename, const string& commentChars = "#", int verbosity = 0);


/**
* @brief Retrieves a substring between two specified delimiters in a string.
*
* This function extracts and returns the substring found between the first and second delimiters within the input string.
*
* @param input The input string containing the delimiters.
* @param firstDelimiter The delimiter marking the start of the substring.
* @param secondDelimiter The delimiter marking the end of the substring.
* @return The substring found between the specified delimiters, or an empty string if delimiters are not found.
*/
string retrieveStringBetweenChars(const string& input, const string& firstDelimiter, const string& secondDelimiter);


/**
* @brief Splits a string into a vector of substrings using a specified delimiter.
*
* This function splits the input string by the given single-character delimiter and returns the substrings as a vector.
*
* @param input The input string to split.
* @param x The delimiter character used to split the string.
* @return A vector of strings containing the split substrings.
*/
vector<string> getStringVectorFromStringWithDelimiter(const string& input, const string& x);

/**
* @brief Checks if a directory exists at the given path.
*
* This function checks the file system to determine whether a directory exists at the specified path.
*
* @param path The path to check for a directory.
* @return `true` if the directory exists, `false` otherwise.
*/
bool directoryExists(const std::string& path);


/**
* @brief Searches for a directory within a list of possible locations.
*
* This function iterates over a list of possible locations to find the first directory that matches the specified name.
*
* @param dirName The name of the directory to search for.
* @param possibleLocations A vector of paths where the directory might be located.
* @return The path to the found directory or "UNINITIALIZEDSTRINGQUANTITY" if not found.
*/
string searchForDirInLocations(const string& dirName, const vector<string>& possibleLocations);


/**
* @brief Checks if a filename has one of the specified extensions.
*
* This function checks whether a given filename ends with any of the extensions specified in a list.
*
* @param filename The name of the file to check.
* @param extensions A vector of extensions to check against.
* @return `true` if the filename has one of the specified extensions, `false` otherwise.
*/
bool hasExtension(const string& filename, const vector<string>& extensions);

/**
* @brief Retrieves a list of files with specific extensions from a directory.
*
* This function opens the specified directory and retrieves a list of files that match the given extensions.
*
* @param dirName The name of the directory to search in.
* @param extensions A vector of file extensions to filter the files.
* @return A vector of file names that have the specified extensions.
*/
vector<string> getListOfFilesInDirectory(const string& dirName, const vector<string>& extensions);

/**
* @brief Converts a string to lowercase.
*
* This function transforms all characters in the input string to lowercase.
*
* @param str The string to convert.
* @return The lowercase version of the input string.
*/
string convertToLowercase(const string& str);

/**
* @brief Retrieves all keys from a map.
*
* This function extracts and returns all the keys from the given map.
*
* @tparam KEY The type of keys in the map.
* @tparam VALUE The type of values in the map.
* @param map The map from which to extract keys.
* @return A vector containing all the keys from the map.
*/
template <class KEY, class VALUE>
vector<KEY> getKeys(const map<KEY, VALUE>& map);

/**
 * @brief Enumeration of random models.
 */
enum randomModel
{
    uniform,  ///< Uniform distribution
    gaussian, ///< Gaussian distribution
    cosine,   ///< Cosine distribution
    sphere    ///< Sphere distribution
};

/**
* @brief Converts a string to a corresponding `randomModel` enum value.
*
* This function maps a given string to its corresponding `randomModel` enum value.
* If the string does not match any known model, an exception is thrown.
*
* @param str The string representation of the random model.
* @return The corresponding `randomModel` enum value.
* @throws std::invalid_argument if the string does not match any valid model.
*/
randomModel stringToRandomModel(const std::string& str);

inline constexpr const char* to_string(randomModel m) noexcept {
    switch (m) {
    case uniform: return "uniform";
    case gaussian: return "gaussian";
    case cosine: return "cosine";
    case sphere: return "sphere";
    }
    return "<unknown>";
}

/**
 * @brief Convert a hex colour string to G4Colour.
 *
 * Accepted formats (case‑insensitive):
 *   • "#RRGGBB"  or "RRGGBB"   – opaque          (six hex digits)
 *   • "#RRGGBBT" or "RRGGBBT" – with 1‑digit α  (seven hex digits; 0 = fully transparent, F = opaque)
 *
 * No allocations, one integer parse, and throws std::invalid_argument on malformed input.
 */
G4Colour makeG4Colour(std::string_view code, double opacity);
};


#include <filesystem>

#if defined(__APPLE__)
#include <mach-o/dyld.h>   // _NSGetExecutablePath
#elif defined(__linux__)
#include <unistd.h>        // readlink
#include <limits.h>
#elif defined(_WIN32)
#include <windows.h>
#include <vector>
#else
#error "Unsupported platform"
#endif

namespace gutilities {
inline std::filesystem::path executable_path() {
#if defined(__APPLE__)
    char     buf[PATH_MAX];
    uint32_t sz = sizeof(buf);
    if (_NSGetExecutablePath(buf, &sz) != 0) {
        // buffer too small
        std::string big(sz, '\0');
        if (_NSGetExecutablePath(big.data(), &sz) != 0)
            throw std::runtime_error("_NSGetExecutablePath failed");
        return std::filesystem::canonical(big);
    }
    return std::filesystem::canonical(buf);

#elif defined(__linux__)
    char    buf[PATH_MAX];
    ssize_t len = ::readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len == -1)
        throw std::runtime_error("readlink(/proc/self/exe) failed");
    buf[len] = '\0';
    return std::filesystem::canonical(buf);

#elif defined(_WIN32)
    std::wstring buf(MAX_PATH, L'\0');
    DWORD        len = ::GetModuleFileNameW(nullptr, buf.data(), buf.size());
    if (len == 0)
        throw std::runtime_error("GetModuleFileNameW failed");
    // If the path is longer than MAX_PATH the buffer is truncated;
    // do a second call with the returned length to get the full path.
    if (len == buf.size()) {
        buf.resize(len * 2);
        len = ::GetModuleFileNameW(nullptr, buf.data(), buf.size());
    }
    buf.resize(len);
    return std::filesystem::canonical(std::filesystem::path(buf));
#endif
}

inline std::filesystem::path gemc_root() {
    const auto exe_dir = executable_path().parent_path();

    std::filesystem::path root;

    // Case 1: executable came from ../bin or (for tests) build
    if (exe_dir.filename() == "bin" || exe_dir.filename() == "build") {
        root = exe_dir.parent_path();
    }
    // else {
    //     // Case 2: use GEMC environment variable
    //     const char* env = std::getenv("GEMC");
    //     if (!env || std::string(env).empty()) {
    //         throw std::runtime_error(
    //             "GEMC executable not in <.../bin>. Environment variable GEMC is required."
    //         );
    //     }
    //     root = std::filesystem::path(env);
    // }

    // Sanity check
    if (!std::filesystem::exists(root / "api")) {
        throw std::runtime_error(
            "Cannot locate directory <api> under " + root.string() +
            ". Check installation layout or GEMC environment variable."
        );
    }

    return root;
}


bool is_unset(std::string_view s);

inline std::string success_or_fail(bool condition) { return condition ? "success" : "fail"; }
void               apply_uimanager_commands(const std::string& commands);
}
