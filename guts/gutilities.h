#ifndef  GSTRING_H
#define  GSTRING_H  1

// conventions
#include "gutsConventions.h"

// c++
#include <vector>
#include <string>
#include <map>

namespace gutilities {

	using std::string;
	using std::vector;
	using std::map;

	// Removes leading and trailing spaces
	string removeLeadingAndTrailingSpacesFromString(string);

	// Removes all spaces from string
	string removeAllSpacesFromString(string);


	// get the filename from the path (currently only posix)
	string getFileFromPath(string);
	
	// get the directory from the path (currently only posix)
	string getDirFromPath(string);

	// a vector of strings from a stringstream, space is delimiter
	vector<string> getStringVectorFromString(string);

	// returns a vector of strings from a stringstream, second string is delimiter
	vector<string> getStringVectorFromStringWithDelimiter(string, string);

	// Replace all occurences of a char in string with a string
	string replaceCharInStringWithChars(string input, string toReplace, string replacement);

	// Replace all occurences of a a string with a string
	string replaceAllStringsWithString(string source, const string from, const string to);

	// adds characters to a word to fill its digits up to ndigits
	string fillDigits(string word, string c, int ndigits);

	// gets a g4number from a string that contains units
	double getG4Number(string input, bool warnIfNotUnit = false);

	double getG4Number(double input, string unit);

	// gets a vector of g4numbers from a vector of strings containing units
	vector<double> getG4NumbersFromStringVector(vector<string> vstring, bool warnIfNotUnit = false);

	// gets a vector of g4numbers from a strings containing units
	vector<double> getG4NumbersFromString(string vstring, bool warnIfNotUnit = false);

	// parse a file, remove comments (starting with commentChars) and return the pure JSON string
	string parseFileAndRemoveComments(string filename, string commentChars = "#", int verbosity = 0);

	// retrieve string between two strings
	string retrieveStringBetweenChars(string input, string firstDelimiter, string secondDelimiter);

	// retrieve string between two strings
	string retrieveStringBetweenChars(string input, string firstDelimiter, string secondDelimiter);

	// string search for a path with <name> from a possible list of absolute paths
	// returns UNINITIALIZEDSTRINGQUANTITY if not found
	string searchForDirInLocations(string dirName, vector<string> possibleLocations);

	vector<string> getListOfFilesInDirectory(string dirName, vector<string> extensions) ;

	template<class KEY, class VALUE> vector<KEY> getKeys(const map<KEY, VALUE>& map);

};


#endif
