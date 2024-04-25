#ifndef GROOTTREE_H
#define GROOTTREE_H 1

// c++
#include <map>

// ROOT
#include "TTree.h"

// glibrary
#include "event/gEventDataCollection.h"

#define TRUEINFONAMEPREFIX   "trueInfo_"
#define DIGITIZEDNAMEPREFIX  "digitized_"

#define EC__GSTREAMERROOTTREENOTFOUND   850

#define HEADERTREENAME "header"

#define HEADERTREENAMESUFFIX    "GEMC Root Event Header"
#define TRUEINFOTREENAMESUFFIX  "True Info Data Root Tree"
#define DIGITIZEDTREENAMESUFFIX "Digitized Data Root Tree"

// connection between gdata and root
class GRootTree
{
public:

	// types of TTree
	GRootTree(const GEventDataCollectionHeader *gheader);
	GRootTree(const string detectorName, const GTrueInfoData* gdata);
	GRootTree(const string detectorName, const GDigitizedData* gdata);

	// filling trees
	bool fillTree(const GEventDataCollectionHeader *gheader);
	bool fillTree(const vector<GTrueInfoData*>*  trueInfoData);
	bool fillTree(const vector<GDigitizedData*>* digitizedData);

	// clear variables map below
	bool initTreeForTheEvent();

private:
	TTree *rootTree = nullptr;

	// variable maps
	// index is hit number
	map<string, vector<int>* >    intVarsMap;
	map<string, vector<float>* >  floatVarsMap;
	map<string, vector<double>* > doubleVarsMap;
	map<string, vector<string>* > stringVarsMap;


	// the second argument is needed to select the VarsMap type and its content
	void registerVariable(const string varname, const int value);
	void registerVariable(const string varname, const float value);
	void registerVariable(const string varname, const double value);
	void registerVariable(const string varname, const string value);

};

#endif // GROOTTREE_H

