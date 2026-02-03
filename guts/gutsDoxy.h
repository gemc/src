/**
 *  \mainpage
 *
 *  \tableofcontents
 *
 *  \section overview Overview
 *  The **gutilities** module provides a suite of small, dependency-light helpers grouped under
 *  the \ref gutilities "gutilities" namespace. These utilities are reused across the codebase
 *  to keep common operations (string normalization, filesystem probing, numeric parsing) consistent
 *  and easy to audit.
 *
 *  The guiding design goals are:
 *  - **Small surface area**: each function does one thing, with explicit behavior.
 *  - **Predictable parsing**: numeric parsing is designed to be locale-independent.
 *  - **Reuse**: common patterns live in one place rather than being re-implemented in multiple modules.
 *
 *  \section namespace_contents Main namespace contents
 *  The \ref gutilities "gutilities" namespace includes (non-exhaustive):
 *
 *  - \ref gutilities::removeLeadingAndTrailingSpacesFromString "removeLeadingAndTrailingSpacesFromString()"
 *    - Two overloads: an owning \c std::string variant and a \c std::string_view variant.
 *  - \ref gutilities::getStringVectorFromString "getStringVectorFromString()"
 *    - Tokenizes whitespace-separated text into a vector of trimmed tokens.
 *  - \ref gutilities::getFileFromPath "getFileFromPath()"
 *    - Extracts the last path component from a POSIX-style path.
 *  - \ref gutilities::getDirFromPath "getDirFromPath()"
 *    - Extracts the directory portion of a POSIX-style path.
 *  - \ref gutilities::getG4Number "getG4Number()"
 *    - Parses \c "<number>*<unit>" and converts to internal units when recognized.
 *
 *  \section conventions Conventions and shared macros
 *  The module follows conventions defined in gutsConventions.h :
 *
 *  - **Message prefixes**:
 *    - \c FATALERRORL and \c GWARNING provide standardized fatal/warning prefixes.
 *  - **Text formatting**:
 *    - ANSI helpers such as \c FRED("...") or \c BOLD("...") for colored/bold console output.
 *  - **List glyphs**:
 *    - \c POINTITEM, \c CIRCLEITEM, \c SQUAREITEM, etc., to keep console logs consistent.
 *
 *  \section examples Examples
 *
 *  \subsection ex_string_vector Tokenizing a string into words
 *  The example program \c examples/string_vector_from_string.cc demonstrates how to use
 *  \ref gutilities::getStringVectorFromString "getStringVectorFromString()" to split a single
 *  command-line argument into tokens and print them.
 *
 *  ~~~~cpp
 *  #include <iostream>
 *  #include <vector>
 *  #include "gutilities.h"
 *
 *  int main(int argc, char* argv[]) {
 *      std::string input = "strings separated by spaces";
 *      std::vector<std::string> result = gutilities::getStringVectorFromString(input);
 *
 *      for (const auto& word : result) {
 *          std::cout << word << std::endl;
 *      }
 *      return 0;
 *  }
 *  ~~~~
 *
 *  Example output:
 *  \code{.bash}
 *  strings
 *  separated
 *  by
 *  spaces
 *  \endcode
 *
 *  \section additional_features Additional features
 *  Additional utility functions include:
 *
 *  - \ref gutilities::replaceCharInStringWithChars "replaceCharInStringWithChars()"
 *    - Replaces any of a set of characters with a replacement string.
 *  - \ref gutilities::directoryExists "directoryExists()"
 *    - Tests whether a directory exists at a path.
 *  - \ref gutilities::getListOfFilesInDirectory "getListOfFilesInDirectory()"
 *    - Scans a directory for regular files with matching extensions.
 *
 *  \section ownership Ownership and maintenance
 *  This module is maintained as part of the broader codebase by the primary project maintainer.
 *  When updating these utilities, prefer:
 *  - adding documentation for behavior changes,
 *  - keeping parsing rules explicit and testable,
 *  - avoiding implicit dependencies on platform locale and environment.
 *
 *  \n\n
 *  \author \n &copy; Maurizio Ungaro
 *  \author e-mail: ungaro@jlab.org
 *  \n\n\n
 */
