#ifndef GUTILITIES_H
#define GUTILITIES_H 1

// GOptions
// --------

// returns array of options definitions
vector<GOption> defineOptions();
bool getGui(GOptions *gopts);



	
// qt
#include <QApplication>

// distinguishing between graphical and batch mode
QCoreApplication* createQtApplication(int &argc, char *argv[], bool gui);

#endif
