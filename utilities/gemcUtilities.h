#ifndef GUTILITIES_H
#define GUTILITIES_H 1

// qt
#include <QApplication>

// glibrary
#include "goptions.h"

// geant4
#include "G4MTRunManager.hh"

// distinguishing between graphical and batch mode
QCoreApplication* createQtApplication(int &argc, char *argv[], bool gui);

// return number of cores from options. If 0 or none given,
// returns max number of available cores
int getNumberOfThreads(GOptions* gopts);

// initialize G4MTRunManager
void initGemcG4RunManager(G4MTRunManager *grm, GOptions* gopt);

// - batch
// - gui (if needed)
vector<string> startingUIMCommands(bool gui);

// apply initial UIM commands coming from, in order:
// - startingUIMCommands
// - PRAGMA TODO macro file
void applyInitialUIManagerCommands(bool gui, int verbosity);

#endif
