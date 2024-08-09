/// \mainpage
/// \section Overview
/// The **gutilities** library provides a suite of basic utilities encapsulated within a dedicated namespace,
/// offering a range of functionality for string manipulation and console output formatting.
///
/// \section Main gutilities namespace:
/// The `gutilities` namespace includes essential utility functions such as:
///
/// - `string trimSpacesFromString(string input)`
///   - Removes leading and trailing spaces from the given string.
/// - `vector<string> getStringVectorFromString(string input)`
///   - Splits a string into a vector of substrings based on spaces or other delimiters.
///
/// \section Conventions
/// The **gutilities** library adheres to several conventions for consistent output formatting and verbosity:
///
/// - **Color Macros**:
///   - Utilize macros to format console output with colors, e.g., `FRED("my sentence")` to print in red.
/// - **Verbosity Levels**:
///   - Control the level of detail in logging output with verbosity levels ranging from 0 to 3.
/// - **Predefined Strings**:
///   - Use predefined string macros such as `FATALERRORL`, `GWARNING`, `POINTITEM`, `CIRCLEITEM`, and `SQUAREITEM`
///     to standardize message outputs and highlight key information.
///
/// \section Example
/// The following example demonstrates how to use `getStringVectorFromString` to split a string into individual words:
///
/// ~~~~cpp
/// #include <iostream>
/// #include <vector>
/// #include "gutilities.h" // Ensure this includes the header file where these utilities are defined
///
/// int main() {
///     std::string input = "strings separated by spaces";
///     std::vector<std::string> result = gutilities::getStringVectorFromString(input);
///
///     for (const auto& word : result) {
///         std::cout << word << std::endl;
///     }
///
///     return 0;
/// }
/// ~~~~
///
/// **Usage**:
///
/// Run the above code with a string input to see each word printed on a new line:
///
/// \code{.bash}
/// > example "strings separated by spaces"
/// strings
/// separated
/// by
/// spaces
/// \endcode
