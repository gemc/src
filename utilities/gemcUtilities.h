#ifndef GUTILITIES_H
#define GUTILITIES_H 1

// qt
#include <QApplication>

// glibrary
#include "goptions.h"
#include "gdynamicdigitization.h"

// geant4
#include "G4MTRunManager.hh"

// distinguishing between graphical and batch mode
QCoreApplication* createQtApplication(int &argc, char *argv[], bool gui);

// return number of cores from options. If 0 or none given,
// returns max number of available cores
int getNumberOfThreads(GOptions* gopts);

// initialize G4MTRunManager
void initGemcG4RunManager(G4RunManager *grm, GOptions* gopt);

// - batch
// - gui (if needed)
vector<string> startingUIMCommands(bool gui);

// apply initial UIM commands coming from, in order:
// - startingUIMCommands
// - PRAGMA TODO macro file
void applyInitialUIManagerCommands(bool gui, int verbosity);

// loads plugins from sensitive map <names, paths>
// exits if failure
void loadDigitizationPlugins(GOptions* gopt, vector<string> sdetectors, map<string, GDynamicDigitization*> *gDDGlobal);

string definePluginPath(GOptions* gopts);

#endif
