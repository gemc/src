/**
 *  \mainpage
 *  \section Overview
 *  The **gutilities** library provides a suite of basic utilities encapsulated within a dedicated namespace,
 *  offering a range of functionality for string manipulation, file path operations, and unit conversions.
 * 
 *  \section Main gutilities namespace:
 *  The `gutilities` namespace includes essential utility functions such as:
 * 
 *  - `std::string removeLeadingAndTrailingSpacesFromString(const std::string &input)`
 *    - Removes leading and trailing spaces and tabs from the given string.
 *  - `std::vector<std::string> getStringVectorFromString(const std::string &input)`
 *    - Splits a string into a vector of substrings based on spaces, removing leading and trailing spaces from each.
 *  - `std::string getFileFromPath(const std::string &path)`
 *    - Extracts the filename from a given file path.
 *  - `std::string getDirFromPath(const std::string &path)`
 *    - Extracts the directory path from a given file path.
 *  - `double getG4Number(const std::string &v, bool warnIfNotUnit = false)`
 *    - Converts a string representation of a number with optional units to a double, with an optional warning if no units are specified.
 * 
 *  \section Conventions
 *  The **gutilities** library adheres to several conventions for consistent output formatting and verbosity:
 * 
 *  - **Color Macros**:
 *    - Utilize macros to format console output with colors, e.g., `FRED("my sentence")` to print in red.
 *  - **Verbosity Levels**:
 *    - Control the level of detail in logging output with verbosity levels ranging from 0 to 3.
 *  - **Predefined Strings**:
 *    - Use predefined string macros such as `FATALERRORL`, `GWARNING`, `POINTITEM`, `CIRCLEITEM`, and `SQUAREITEM`
 *      to standardize message outputs and highlight key information.
 * 
 *  \section Example
 *  The following example demonstrates how to use `getStringVectorFromString` to split a string into individual words:
 * 
 *  ~~~~cpp
 *  #include <iostream>
 *  #include <vector>
 *  #include "gutilities.h" // Ensure this includes the header file where these utilities are defined
 * 
 *  int main() {
 *      std::string input = "strings separated by spaces";
 *      std::vector<std::string> result = gutilities::getStringVectorFromString(input);
 * 
 *      for (const auto& word : result) {
 *          std::cout << word << std::endl;
 *      }
 * 
 *      return 0;
 *  }
 *  ~~~~
 * 
 *  **Usage**:
 * 
 *  Run the above code with a string input to see each word printed on a new line:
 * 
 *  \code{.bash}
 *  > example "strings separated by spaces"
 *  strings
 *  separated
 *  by
 *  spaces
 *  \endcode
 * 
 *  \section Additional Features
 * 
 *  The library provides additional utility functions, such as:
 * 
 *  - `std::string replaceCharInStringWithChars(const std::string &input, const std::string &toReplace, const std::string &replacement)`
 *    - Replaces all occurrences of specified characters in a string with another string.
 *  - `bool directoryExists(const std::string &path)`
 *    - Checks if a directory exists at the given path.
 *  - `std::vector<std::string> getListOfFilesInDirectory(const std::string &dirName, const std::vector<std::string> &extensions)`
 *    - Retrieves a list of files with specific extensions from a directory.
 * 
 *  These utilities are designed to streamline common tasks in software development by providing efficient
 *  and easy-to-use functions for string manipulation and file system operations.
 *
 * \subsection cisubsection Continuous Integration
 * The GOptions framework is continuously integrated and tested to ensure stability and reliability across updates.
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n
 */
