// gutilities
#include "gutilities.h"
#include "gutsConventions.h"

// c++
#include <iostream>
using namespace std;


/**
 * @file string_vector_from_string.cc
 * @brief Example program demonstrating tokenization via gutilities helpers.
 *
 * This example reads a single command-line argument containing a whitespace-separated list
 * of tokens (typically provided in quotes) and prints each token on its own line.
 *
 * It specifically demonstrates:
 * - \ref gutilities::getStringVectorFromString "getStringVectorFromString()"
 * - Use of formatting glyphs from gutsConventions.h : (e.g., \c SQUAREITEM)
 */

/**
 * @brief Program entry point.
 *
 * Expected usage:
 * - Exactly one argument containing a whitespace-separated list of tokens.
 *
 * Behavior:
 * - If the argument count is incorrect, prints a fatal-style message and returns failure.
 * - Otherwise tokenizes the string and prints the resulting vector.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return \c EXIT_SUCCESS on success, \c EXIT_FAILURE on incorrect usage.
 */
int main(int argc, char* argv[]) {
	if (argc != 2) {
		cerr << FATALERRORL << "run example with exactly one argument: a quotes enclosed list of strings." << endl;
		return EXIT_FAILURE;
	}
	else {
		string         test       = argv[1];
		vector<string> testResult = gutilities::getStringVectorFromString(test);

		cout << " Original String: " << test << endl;
		cout << " List of items in string: " << endl;
		for (auto& s : testResult) {
			cout << SQUAREITEM << s << endl;
		}
	}

	return EXIT_SUCCESS;
}
