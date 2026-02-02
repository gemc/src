/**
 *  \mainpage
 *
 *  \section Overview
 *  The **gutilities** library provides a suite of utility helpers grouped under the \ref gutilities "gutilities"
 *  namespace. The focus is on small, dependency-light building blocks that are reused across the codebase:
 *
 *  - String normalization (trimming, tokenizing, replacements)
 *  - Path manipulation and basic directory scanning
 *  - Parsing numeric strings with optional units into CLHEP/Geant4 internal units
 *  - Small Geant4 convenience helpers (e.g., applying UI commands, constructing \c G4Colour)
 *
 *  \section Main namespace contents
 *  The \ref gutilities "gutilities" namespace includes:
 *
 *  - \ref gutilities::removeLeadingAndTrailingSpacesFromString "removeLeadingAndTrailingSpacesFromString()"
 *    - Two overloads: one returning an owning \c std::string and one operating on \c std::string_view.
 *  - \ref gutilities::getStringVectorFromString "getStringVectorFromString()"
 *    - Tokenizes a whitespace-separated string into a vector.
 *  - \ref gutilities::getFileFromPath "getFileFromPath()"
 *    - Extracts the last path component from a POSIX-style path.
 *  - \ref gutilities::getDirFromPath "getDirFromPath()"
 *    - Extracts the directory portion from a POSIX-style path.
 *  - \ref gutilities::getG4Number "getG4Number()"
 *    - Parses \c "<number>*<unit>" and converts to CLHEP internal units when recognized.
 *
 *  \section Conventions
 *  The **gutilities** library follows conventions defined in gutsConventions.h :
 *
 *  - **Message prefixes**:
 *    - \c FATALERRORL and \c GWARNING provide standardized fatal/warning prefixes.
 *  - **Text formatting**:
 *    - ANSI helpers such as \c FRED("...") or \c BOLD("...") for colored/bold console output.
 *  - **List glyphs**:
 *    - \c POINTITEM, \c CIRCLEITEM, \c SQUAREITEM, etc., to keep console logs consistent.
 *
 *  \section Example
 *  The following example demonstrates how to use
 *  \ref gutilities::getStringVectorFromString "getStringVectorFromString()"
 *  to split a string into individual words:
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
 *
 *      return 0;
 *  }
 *  ~~~~
 *
 *  **Usage**:
 *
 *  \code{.bash}
 *  > example "strings separated by spaces"
 *  strings
 *  separated
 *  by
 *  spaces
 *  \endcode
 *
 *  \section Additional features
 *  Additional utility functions include:
 *
 *  - \ref gutilities::replaceCharInStringWithChars "replaceCharInStringWithChars()"
 *    - Replaces any of a set of characters with a replacement string.
 *  - \ref gutilities::directoryExists "directoryExists()"
 *    - Tests whether a directory exists at a path.
 *  - \ref gutilities::getListOfFilesInDirectory "getListOfFilesInDirectory()"
 *    - Scans a directory for regular files with matching extensions.
 *
 *  \subsection cisubsection Continuous Integration
 *  The gutilities code is continuously integrated and tested to ensure stability and reliability across updates.
 *
 *  \n\n
 *  \author \n &copy; Maurizio Ungaro
 *  \author e-mail: ungaro@jlab.org
 *  \n\n\n
 */
