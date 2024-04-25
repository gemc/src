#ifndef  GMODIFIER_H
#define  GMODIFIER_H 1

// c++
#include <iostream>
using std::string;
using std::cout;
using std::endl;

class GModifier
{
	
public:
	GModifier(string n, string s, string t, bool existance, int verbosity) :
	name(n),
	shift(s),
	tilts(t),
	isPresent(existance) {

		if( verbosity ) {
			cout << GSYSTEMLOGHEADER <<  "volume " << name << " modified with:";
			if (  shift != GSYSTEMNOMODIFIER) cout << " - shift: "     << shift   ;
			if (  tilts != GSYSTEMNOMODIFIER) cout << " - tilts: "     << tilts   ;
			if ( !isPresent )         cout << " - existence: " << isPresent ;
			cout << endl;
		}
	}

private:
	string name;  // volume name
	string shift;
	string tilts;
	bool   isPresent; // true by default

public:
	string getShift()     { return shift; }
	string getTilts()     { return tilts; }
	bool   getExistence() { return isPresent; }
};

#endif
