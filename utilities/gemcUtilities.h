#ifndef GUTILITIES_H
#define GUTILITIES_H 1

// qt
#include <QApplication>


// distinguishing between graphical and batch mode
QCoreApplication* createQtApplication(int &argc, char *argv[], bool gui);

#endif
