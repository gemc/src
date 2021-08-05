#ifndef GUTILITIES_H
#define GUTILITIES_H 1

// qt
#include <QApplication>

// glibrary
#include "goptions.h"

// distinguishing between graphical and batch mode
QCoreApplication* createQtApplication(int &argc, char *argv[], bool gui);

// return number of cores from options. If 0 or none given,
// returns max number of available cores
int getNumberOfThreads(GOptions* gopts);

// initialize digitization map
map<string, GDynamicDigitization*> initializeDigitizationFactoryMap(GOptions *gopts);

#endif
