// gutilities
#include "gutilities.h"

// c++
// algorithm for the erase/remove combo
#include <sstream>
#include<algorithm>
using namespace std;

//! Trim Both leading and trailing spaces
string gutilities::removeLeadingAndTrailingSpacesFromString(string in)
{
	//	str.erase(remove(str.begin(), str.end(), ' '), str.end());
	//	return str;
	
	string out;
	
	size_t leapos = in.find_first_not_of(" \t"); // Find the first character position after excluding leading blank spaces
	size_t endpos = in.find_last_not_of(" \t");  // Find the first character position from reverse af
	
	// if all spaces or empty return an empty string
	if(( leapos == string::npos) || ( endpos == string::npos)) {
		out = "";
	} else {
		out = in.substr( leapos, endpos-leapos+1 );
	}
	return out;
	
}


//! Remove all spaces from string
string gutilities::removeAllSpacesFromString(string str)
{
	str.erase(remove(str.begin(), str.end(), ' '), str.end());
	return str;
}

//! Get the filename from the path (currently only posix)
string gutilities::getFileFromPath(string path)
{
	return path.substr(path.find_last_of("/") + 1);
}

//! Get the directory from the path (currently only posix)
string gutilities::getDirFromPath(string path)
{
	auto lastSlash = path.find_last_of("/");
	return path.substr(0, lastSlash);
}

//! Return a vector of strings from a stringstream, space is delimeter
vector<string> gutilities::getStringVectorFromString(string input)
{
	vector<string> pvalues;
	stringstream plist(input);
	while(!plist.eof()) {
		string tmp;
		plist >> tmp;
		string toPut = removeLeadingAndTrailingSpacesFromString(tmp);
		if(toPut != "")
			pvalues.push_back(toPut);
	}
	
	return pvalues;
}

//! Replace all occurences of specific chars in a string with a string
string gutilities::replaceCharInStringWithChars(string input, string toReplace, string replacement)
{
	
	string output = "";
	
	for(unsigned int k=0; k<input.size(); k++) {
		
		int replace = 0;
		
		for(unsigned int j=0; j<toReplace.size(); j++) {
			// found common char, replacing it with replacement
			if(input[k] == toReplace[j]) {
				output.append(replacement);
				replace = 1;
			}
		}
		if(!replace) output += input[k];
	}
	
	return output;
}

//! Replace all occurences of a a string with a string
string gutilities::replaceAllStringsWithString(string source, const string from, const string to)
{
	string newString;
	
	size_t lastPos = 0;
	size_t findPos;
	
	while((findPos = source.find(from, lastPos) != string::npos)) {
		newString += to;
		newString.append(source, lastPos + from.length(), findPos - lastPos  );
		lastPos = findPos + from.length();
	}
	
	// Care for the rest after last occurrence
	newString += source.substr(lastPos);
	
	return newString;
}


//! Fill a string with the string c to be ndigits long
string gutilities::fillDigits(string word, string c, int ndigits)
{
	string filled;
	
	int toFill = ndigits - (int) word.size();
	
	for(int d=0; d<toFill; d++) {
		filled += c;
	}
	
	filled += word;
	
	return filled;
}




// Numbers / strings with units / io

// CLHEP units
#include "CLHEP/Units/PhysicalConstants.h"
using namespace CLHEP;

// c++
#include <iostream>

/// \fn double getG4Number(string v, bool warnIfNotUnit)
/// \brief Return value of the input string, which may or may not
/// contain units (warning given if requested)
/// \param v input string. Ex: 10.2*cm
/// \return value with correct G4 unit.
double gutilities::getG4Number(string v, bool warnIfNotUnit)
{
	string value = removeLeadingAndTrailingSpacesFromString(v);
	
	// no * found
	if(value.find("*") == string::npos) {
		// no * found, this should be a number
		// no unit is still ok if the number is 0 or if it's meant to be a count
		
		if(value.length()>0 && warnIfNotUnit && stod(value) != 0) {
			cerr << " ! Warning: value " << v << " does not contain units." << endl;
		}
		
		double answer = 0;
		
		try {
			answer = stod(value);
		}
		catch(exception& e) {
			cerr << FATALERRORL << "stod exception in gutilities: could not convert string to double. ";
			cerr << "Value: >" << v << "<, error: " << e.what() << endl;
			gexit(EC__G4NUMBERERROR);
		}
		
		return answer;
		
	} else {
		string rootValue = value.substr(0, value.find("*"));
		string units     = value.substr(value.find("*") + 1);
		
		double answer = 0;
		
		try {
			answer = stod(rootValue);
		}
		catch(exception& e) {
			cerr << FATALERRORL << "stod exception in gutilities: could not convert string to double. ";
			cerr << "Value: >" << v << "<, error: " << e.what() << endl;
			gexit(EC__G4NUMBERERROR);
		}
		
		
		if( units == "m")              answer *= m;
		else if( units == "inches")    answer *= 2.54*cm;
		else if( units == "inch")      answer *= 2.54*cm;
		else if( units == "cm")        answer *= cm;
		else if( units == "mm")        answer *= mm;
		else if( units == "um")        answer *= 1E-6*m;
		else if( units == "fm")        answer *= 1E-15*m;
		else if( units == "deg")       answer *= deg;
		else if( units == "degrees")   answer *= deg;
		else if( units == "arcmin")    answer = answer/60.0*deg;
		else if( units == "rad")       answer *= rad;
		else if( units == "mrad")      answer *= mrad;
		else if( units == "eV")        answer *= eV;
		else if( units == "MeV")       answer *= MeV;
		else if( units == "KeV")       answer *= 0.001*MeV;
		else if( units == "GeV")       answer *= GeV;
		else if( units == "T")         answer *= tesla;
		else if( units == "T/m")       answer *= tesla/m;
		else if( units == "Tesla")     answer *= tesla;
		else if( units == "gauss")     answer *= gauss;
		else if( units == "kilogauss") answer *= gauss*1000;
		else if( units == "ns")        answer *= ns;
		else if( units == "ms")        answer *= ms;
		else if( units == "us")        answer *= us;
		else if( units == "counts")    answer *= 1;
		else cerr << GWARNING << ">" << units << "<: unit not recognized for string <" << v << ">" << endl;
		return answer;
	}
	
}



double gutilities::getG4Number(double input, string unit)
{
	string gnumber = to_string(input) + "*" + unit;
	return  getG4Number(gnumber);
}

vector<double> gutilities::getG4NumbersFromStringVector(vector<string> vstring, bool warnIfNotUnit)
{
	vector<double> output;
	
	for(auto &s: vstring) {
		output.push_back(getG4Number(s, warnIfNotUnit));
	}
	
	return output;
}

vector<double>  gutilities::getG4NumbersFromString(string vstring, bool warnIfNotUnit) {
	return getG4NumbersFromStringVector(getStringVectorFromStringWithDelimiter(vstring, ","), warnIfNotUnit);
}


// c++
#include <fstream>

// need to add verbosity
string gutilities::parseFileAndRemoveComments(string filename, string commentChars, int verbosity) {
	
	// reading file
	stringstream strStream;
	ifstream in(filename.c_str());
	if(!in) {
		cerr << FATALERRORL << "can't open input file " << filename << ". Check your spelling. " << endl;
		gexit(EC__FILENOTFOUND);
	} else {
		if(verbosity > 0) {
			cout << endl << CIRCLEITEM <<  " Loading string from " << filename << endl;
		}
		strStream << in.rdbuf(); //read the file
	}
	in.close();
	
	
	string parsedString = strStream.str();
	
	// removing all occurances of commentChars
	while (parsedString.find(commentChars.c_str()) !=  string::npos ) {
		size_t nFPos = parsedString.find(commentChars.c_str());   // locate commentChars in the string
		size_t secondNL = parsedString.find('\n', nFPos);         // locate the next CR starting from where commentChars was found
		size_t firstNL = parsedString.rfind('\n', nFPos);         // locate the last CR before where commentChars was found
		
		// remove the lines containing the comment
		parsedString.erase(firstNL, secondNL - firstNL);
		
	}
	
	return parsedString;
}

// retrieve string between two strings
string gutilities::retrieveStringBetweenChars(string input, string firstDelimiter, string secondDelimiter)
{
	string out;
	
	size_t firstpos  = input.find(firstDelimiter);  // Find the first character position after excluding leading blank spaces
	size_t secondpos = input.find(secondDelimiter); // Find the second character position after excluding leading blank spaces
	
	// if all spaces or empty return an empty string
	if(( firstpos == string::npos) || ( secondpos == string::npos))
		out = "";
	else
		out = input.substr( firstpos + 1, secondpos - firstpos - 1 );
	
	return out;
	
}

// returns a vector of strings from a stringstream, x (one char) is delimiter
vector<string> gutilities::getStringVectorFromStringWithDelimiter(string input, string x)
{
	vector<string> pvalues;
	
	string tmp = "";
	for(unsigned int i=0; i<input.size(); i++) {
		
		if(input[i] != x[0]) {
			tmp += input[i];
		} else {
			if(tmp != "") {
				pvalues.push_back(removeLeadingAndTrailingSpacesFromString(tmp));
			}
			tmp = "";
		}
		
		// end of line
		if(i==input.size() - 1 && tmp != "") {
			pvalues.push_back(removeLeadingAndTrailingSpacesFromString(tmp));
		}
	}
	
	return pvalues;
}

// string search for a path with <name> from a possible list of absolute paths
// returns UNINITIALIZEDSTRINGQUANTITY if not found
#include <filesystem>
namespace fs = std::filesystem;

string gutilities::searchForDirInLocations(string dirName, vector<string> possibleLocations) {
	
	for(auto trialLocation : possibleLocations) {
		string possibleDir = trialLocation + "/" + dirName;
		fs::path path = possibleDir;
		if ( fs::exists(path) ) {
			return possibleDir;
		}
	}
	return UNINITIALIZEDSTRINGQUANTITY;
}

vector<string> gutilities::getListOfFilesInDirectory(string dirName, vector<string> extensions) {
	
	vector<string> fileList;
	
	for (const auto & entry : fs::directory_iterator(dirName)) {
		for ( auto& extension: extensions ) {
			if ( entry.path().extension() == extension ) {
				fileList.push_back(entry.path().filename());
			}
		}
	}
	
	return fileList;
}


// returns all keys from a map<key, value>
// I think this is in STL now?
// wow including this in a code makes the library not build correctly?
// How? I added it in gworld.cc to get first key and it was not building the constructor
template<class KEY, class VALUE> vector<KEY> gutilities::getKeys(const map<KEY, VALUE>& map) {
	vector<KEY> keys(map.size());
	for (const auto& it : map)
		keys.push_back(it.first);
	return keys;
}
