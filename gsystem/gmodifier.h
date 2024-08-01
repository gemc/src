#ifndef  GMODIFIER_H
#define  GMODIFIER_H 1

// c++
#include <iostream>

using std::string;
using std::cout;
using std::endl;

class GModifier {

public:

    GModifier(string n, string s, string t, bool existence, int verbosity) :
            name(n),
            shift(s),
            tilts(t),
            isPresent(existence) {

        if (verbosity) {
            cout << GSYSTEMLOGHEADER << "volume " << name << " modified with:" << KGRN;
            if (shift != GSYSTEMNOMODIFIER) cout << " - shift: " << shift;
            if (tilts != GSYSTEMNOMODIFIER) cout << " - tilts: " << tilts;
            if (!isPresent) cout << " - existence: " << isPresent;
            cout << RST << endl;
        }
    }

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

#endif
