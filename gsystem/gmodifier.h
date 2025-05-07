#pragma once

// c++
#include <iostream>

using std::string;
using std::cout;
using std::endl;

class GModifier {

public:

	GModifier(string n, string s, string t, bool existence) :
			name(n),
			shift(s),
			tilts(t),
			isPresent(existence) {}

	GModifier(const GModifier &other) = default;

private:
	string name;  // volume name
	string shift;
	string tilts;
	bool isPresent; // true by default
	friend ostream &operator<<(ostream &stream, GModifier gm) {
		stream << "GModifier: " << gm.name << " shift: " << gm.shift << " tilts: " << gm.tilts << " isPresent: "
			   << gm.isPresent;
		return stream;
	}

public:
	string getName() { return name; }

	string getShift() { return shift; }

	string getTilts() { return tilts; }

	bool getExistence() { return isPresent; }

};
