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

/**
 * @file gutilities.h
 * @brief Public API for the gutilities namespace.
 *
 * This header declares a set of small utilities used throughout the codebase.
 * Most functions are pure helpers (string/path manipulation, parsing), with a few Geant4-adjacent
 * conveniences (e.g., UI command application and \c G4Colour construction).
 */

namespace gutilities {
using std::string;
using std::vector;
using std::map;

/**
 * @brief Removes leading and trailing spaces and tabs from a string.
 *
 * This overload returns a new owning string after trimming.
 *
 * Whitespace considered:
 * - Space (@c ' ')
 * - Tab (the tab character)
 *
 * If the input contains only whitespace (or is empty), an empty string is returned.
 *
 * @param input The string to be trimmed.
 * @return A new string with leading and trailing spaces and tabs removed.
 *
 * @see removeLeadingAndTrailingSpacesFromString(std::string_view)
 */
std::string removeLeadingAndTrailingSpacesFromString(const std::string& input);

/**
 * @brief Removes leading and trailing whitespace from a string view (no allocation).
 *
 * This overload adjusts the view bounds by removing whitespace from both ends.
 * The returned view references the original underlying buffer.
 *
 * Whitespace classification is based on @c std::isspace.
 *
 * @param input The string_view to be trimmed.
 * @return A trimmed view of the original input.
 */
std::string_view removeLeadingAndTrailingSpacesFromString(const std::string_view input);

/**
 * @brief Removes all spaces from a string.
 *
 * This function removes all literal space characters (@c ' ') from the input string and returns the result.
 * It does not remove other whitespace (tabs/newlines).
 *
 * @param str The input string from which spaces will be removed.
 * @return A new string with all spaces removed.
 */
std::string removeAllSpacesFromString(const std::string& str);

/**
 * @brief Extracts the filename from a given file path.
 *
 * This function extracts and returns the filename from a given POSIX-style file path using @c '/' as separator.
 * If the path contains no separators, the original string is returned.
 *
 * @param path The input file path.
 * @return The filename extracted from the path.
 */
std::string getFileFromPath(const std::string& path);

/**
 * @brief Extracts the directory path from a given file path.
 *
 * This function extracts and returns the directory path from a given POSIX-style file path using @c '/' as separator.
 * If the path contains no separators, @c "." is returned.
 *
 * @param path The input file path.
 * @return The directory path extracted from the path.
 */
std::string getDirFromPath(const std::string& path);

/**
 * @brief Search for a regular file across candidate locations.
 *
 * Each entry in @p locations may be either:
 * - A directory path, in which case @p filename is appended and tested.
 * - A full path to a file candidate, tested directly.
 *
 * The first candidate that exists and is a regular file is returned.
 *
 * @param locations Candidate directories and/or file paths.
 * @param filename Filename to append when probing directory entries.
 * @return Full path string for the first match, or @c std::nullopt if no match is found.
 */
std::optional<std::string> searchForFileInLocations(
	const std::vector<std::string>& locations,
	std::string_view                filename);

/**
 * @brief Splits a string into a vector of strings using whitespace as delimiters.
 *
 * The input is tokenized on whitespace (as implemented by @c std::stringstream extraction).
 * Each token is trimmed (defensively) and empty tokens are skipped.
 *
 * @param input The input string to split.
 * @return A vector of strings obtained by splitting the input string.
 */
std::vector<std::string> getStringVectorFromString(const std::string& input);

/**
 * @brief Replaces all occurrences of specified characters in a string with another string.
 *
 * Every character in @p input is checked against the set of characters in @p toReplace.
 * When a match is found, @p replacement is appended to the output string.
 *
 * @param input The input string in which characters are to be replaced.
 * @param toReplace A string containing characters to be replaced.
 * @param replacement The string to replace each occurrence of characters in @p toReplace.
 * @return A new string with specified characters replaced by the replacement string.
 */
std::string replaceCharInStringWithChars(const std::string& input, const std::string& toReplace,
                                         const std::string& replacement);


/**
 * @brief Replaces all occurrences of a substring with another string.
 *
 * This function performs non-overlapping, left-to-right replacement of all occurrences
 * of @p from with @p to.
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
 * This function prepends the first character of @p c to @p word until its length equals @p ndigits.
 * If @p c is empty or @p ndigits is not larger than the current length, the original @p word is returned.
 *
 * @param word The original string to be padded.
 * @param c The character string to use for padding (only @c c[0] is used).
 * @param ndigits The target length of the padded string.
 * @return A new string padded to the specified length.
 */
string fillDigits(const string& word, const string& c, int ndigits);

/**
 * @brief Converts a string representation of a number with optional units to a double.
 *
 * Accepted formats:
 * - Pure number: @c "12.3"
 * - Number with unit: @c "<number>*<unit>" (exactly one @c '*' separator)
 *
 * Key behavior:
 * - Leading/trailing whitespace is ignored.
 * - A single decimal comma is normalized to a dot if no dot is present.
 * - The numeric part is parsed in the @c "C" locale to avoid locale-dependent decimal separators.
 * - Units are converted using a fixed table and limited SI-prefix handling.
 * - On invalid formatting or numeric parsing errors, the function prints a fatal error and exits with @c EC__G4NUMBERERROR.
 * - If the unit is unknown, a warning is printed and the numeric part is returned (legacy behavior).
 *
 * @param v The input string containing the number and optional units, formatted as @c "<number>*<unit>".
 * @param warnIfNotUnit Flag indicating whether to issue a warning if the units are not present.
 * @return The numeric value converted to a double, taking into account the units if provided.
 */
double getG4Number(const string& v, bool warnIfNotUnit = false);

/**
 * @brief Converts a numeric value with a unit into a Geant4-number string and parses it.
 *
 * This is a convenience overload that constructs @c "<input>*<unit>" and then calls
 * \ref gutilities::getG4Number "getG4Number()".
 *
 * @param input The numeric value to convert.
 * @param unit The unit associated with the numeric value.
 * @return The converted numeric value as a double.
 */
double getG4Number(double input, const string& unit);

/**
 * @brief Converts a vector of strings representing numbers with units to a vector of doubles.
 *
 * Each element is parsed independently using \ref gutilities::getG4Number "getG4Number()".
 *
 * @param vstring A vector of strings, each containing a number with optional units.
 * @param warnIfNotUnit Flag indicating whether to issue a warning if the units are not present.
 * @return A vector of doubles representing the converted numeric values.
 */
vector<double> getG4NumbersFromStringVector(const vector<string>& vstring, bool warnIfNotUnit = false);

/**
 * @brief Converts a comma-separated string of numbers with units to a vector of doubles.
 *
 * The string is split on commas using
 * \ref gutilities::getStringVectorFromStringWithDelimiter "getStringVectorFromStringWithDelimiter()",
 * and each token is parsed using \ref gutilities::getG4Number "getG4Number()".
 *
 * @param vstring A comma-separated string of numbers with optional units.
 * @param warnIfNotUnit Flag indicating whether to issue a warning if the units are not present.
 * @return A vector of doubles representing the converted numeric values.
 */
vector<double> getG4NumbersFromString(const string& vstring, bool warnIfNotUnit = false);

/**
 * @brief Parses a file and removes all lines containing specified comment characters.
 *
 * The entire file is read into a string and then comment lines are removed by treating
 * @p commentChars as the start-of-comment marker.
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
 * If either delimiter is not found, an empty string is returned.
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
 * The delimiter used is @c x[0]. Tokens are trimmed and empty tokens are skipped.
 *
 * @param input The input string to split.
 * @param x The delimiter string (only the first character is used).
 * @return A vector of strings containing the split substrings.
 */
vector<string> getStringVectorFromStringWithDelimiter(const string& input, const string& x);

/**
 * @brief Checks if a directory exists at the given path.
 *
 * @param path The path to check for a directory.
 * @return @c true if the directory exists, @c false otherwise.
 */
bool directoryExists(const std::string& path);


/**
 * @brief Searches for a directory within a list of possible locations.
 *
 * For each candidate base path, the function checks for @c "<base>/<dirName>".
 *
 * @param dirName The name of the directory to search for.
 * @param possibleLocations A vector of paths where the directory might be located.
 * @return The path to the found directory or the literal string @c "UNINITIALIZEDSTRINGQUANTITY" if not found.
 */
string searchForDirInLocations(const string& dirName, const vector<string>& possibleLocations);


/**
 * @brief Checks if a filename has one of the specified extensions.
 *
 * The comparison is a suffix match against each provided extension.
 *
 * @param filename The name of the file to check.
 * @param extensions A vector of extensions to check against.
 * @return @c true if the filename has one of the specified extensions, @c false otherwise.
 */
bool hasExtension(const string& filename, const vector<string>& extensions);

/**
 * @brief Retrieves a list of files with specific extensions from a directory.
 *
 * Only regular files are returned. Directory entries that are not regular files are ignored.
 *
 * @param dirName The name of the directory to search in.
 * @param extensions A vector of file extensions to filter the files.
 * @return A vector of file names that have the specified extensions.
 */
vector<string> getListOfFilesInDirectory(const string& dirName, const vector<string>& extensions);

/**
 * @brief Converts a string to lowercase.
 *
 * @param str The string to convert.
 * @return The lowercase version of the input string.
 */
string convertToLowercase(const string& str);

/**
 * @brief Retrieves all keys from a map.
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
 *
 * This enum is used to select a distribution/model in places where randomized sampling
 * is configurable via human-readable strings.
 */
enum randomModel
{
	uniform,  ///< Uniform distribution
	gaussian, ///< Gaussian distribution
	cosine,   ///< Cosine distribution
	sphere    ///< Sphere distribution
};

/**
 * @brief Converts a string to a corresponding \ref gutilities::randomModel "randomModel" enum value.
 *
 * The mapping is strict and case-sensitive in the current implementation.
 * If the string does not match any known model, an exception is thrown.
 *
 * @param str The string representation of the random model.
 * @return The corresponding \ref gutilities::randomModel "randomModel" enum value.
 * @throws std::invalid_argument if the string does not match any valid model.
 */
randomModel stringToRandomModel(const std::string& str);

/**
 * @brief Convert a \ref gutilities::randomModel "randomModel" enum value to a stable string token.
 *
 * The returned strings match those accepted by \ref gutilities::stringToRandomModel "stringToRandomModel()".
 *
 * @param m Enum value.
 * @return String token for @p m.
 */
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
 * @brief Convert a hex colour string to \c G4Colour.
 *
 * Accepted formats (case-insensitive):
 * - @c "#RRGGBB" or @c "RRGGBB" (six hex digits; leading @c '#' optional)
 *
 * The opacity component is provided explicitly via @p opacity.
 *
 * @param code Hex RGB string.
 * @param opacity Alpha value passed to \c G4Colour.
 * @return Constructed \c G4Colour.
 * @throws std::invalid_argument on malformed input.
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
/**
 * @brief Get the absolute canonical path to the current executable.
 *
 * Platform-specific behavior:
 * - macOS: uses \c _NSGetExecutablePath
 * - Linux: uses \c readlink on @c /proc/self/exe
 * - Windows: uses \c GetModuleFileNameW
 *
 * The returned path is canonicalized via \c std::filesystem::canonical.
 *
 * @return Canonical executable path.
 * @throws std::runtime_error on platform API failures.
 */
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

/**
 * @brief Infer the GEMC installation root directory from the executable location.
 *
 * Current behavior:
 * - If the executable directory is named @c "bin" or @c "build", the root is assumed to be its parent.
 * - The function then requires that @c "<root>/api" exists, otherwise it throws.
 *
 * The environment variable fallback is intentionally commented out and not used.
 *
 * @return Filesystem path to the inferred GEMC root.
 * @throws std::runtime_error if the layout check fails.
 */
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


/**
 * @brief Determine whether a string should be treated as "unset".
 *
 * See the implementation in the corresponding translation unit for the exact matching rules.
 *
 * @param s Input view.
 * @return @c true if @p s represents an "unset" value, @c false otherwise.
 */
bool is_unset(std::string_view s);

/**
 * @brief Convert a boolean condition to a stable status string.
 *
 * @param condition Boolean test.
 * @return @c "success" when @p condition is true; otherwise @c "fail".
 */
inline std::string success_or_fail(bool condition) { return condition ? "success" : "fail"; }

/**
 * @brief Apply a single Geant4 UI command if a UI manager is available.
 *
 * This wrapper obtains the current \c G4UImanager singleton and calls \c ApplyCommand.
 * If no UI manager exists, the call is a no-op.
 *
 * @param commands Command string to apply.
 */
void apply_uimanager_commands(const std::string& commands);
}
