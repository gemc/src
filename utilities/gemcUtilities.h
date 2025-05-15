#pragma once

// qt
#include <QApplication>

// gemc
#include "gemcOptions.h"

// geant4
#include "G4MTRunManager.hh"

// distinguishing between graphical and batch mode
QCoreApplication* createQtApplication(int &argc, char *argv[], bool gui);

// return number of cores from options. If 0 or none given,
// returns max number of available cores
int getNumberOfThreads(GOptions* gopts);

// initialize G4MTRunManager
void initGemcG4RunManager(G4RunManager *grm, GOptions* gopt);

// Starting commands
// - batch
// - gui (if needed), include the commands in batch mode
vector<string> startingUIMCommands(bool gui, int checkForOverlaps);

// apply initial UIM commands coming from, in order:
// - startingUIMCommands
// - PRAGMA TODO macro file
void applyInitialUIManagerCommands(bool gui, int checkForOverlaps, int verbosity);

void startRandomEngine(GOptions* gopts);
