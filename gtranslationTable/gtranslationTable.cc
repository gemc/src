// gtranslationTable 
#include "gtranslationTable.h"
#include "gtranslationTableConventions.h"

// glibrary for gexit
#include "gutsConventions.h"

// c++
#include <iostream>
using namespace std;

string GTranslationTable::formTTKey(vector<int> identity)
{
	string ttKey = "";
	for( size_t v=0; v<identity.size() - 1; v++ ) {
		ttKey += to_string(identity[v]) + "-" ;
	}
	ttKey += to_string(identity.back());

	return ttKey;
}

// TODO: else with warning?
void GTranslationTable::addGElectronicWithIdentity(vector<int> identity, GElectronic gtron)
{
	string ttKey = formTTKey(identity);
	auto search = tt.find(ttKey);

	if(search == tt.end()) {
		tt[ttKey] = gtron;
	} else {
		cout << GWARNING << "Key <" << YELLOWHHL << ttKey << RSTHHR << "> already present in TT map" << endl;
	}
}


GElectronic GTranslationTable::getElectronics(vector<int> identity)
{
	string ttKey = formTTKey(identity);
	auto search = tt.find(ttKey);

	if(search != tt.end()) {
		return search->second;
	} else {
		cerr << FATALERRORL << "Key <" << YELLOWHHL << ttKey << RSTHHR << "> not foud in TT map" << endl;
		gexit(EC__GIDENTITYNOTFOUNDINTT);
	}

	return GElectronic();
}


void GTranslationTable::print()
{
	cout << " Translation Table:" << endl;
	for(auto &thisItem: tt) {
		cout << GTAB << "<" << thisItem.first << "> " << thisItem.second << endl;
	}
}
