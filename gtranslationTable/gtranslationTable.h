#ifndef GTRANSLATIONTABLE_H
#define GTRANSLATIONTABLE_H  1

// glibrary
#include "gelectronic.h"

// c++
#include <string>
#include <map>
#include <vector>
using std::vector;

struct GTranslationTable
{

private:
	// the map uses a string formed by the vector<int> identity as its key
	std::map<std::string, GElectronic> tt;
	std::string formTTKey(vector<int> identity);

public:
	void addGElectronicWithIdentity(vector<int> identity, GElectronic gtron);
	GElectronic getElectronics(const vector<int> identity);
	void print();

};




#endif
