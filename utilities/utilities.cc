// Qt
#include <QtWidgets>

// distinguishing between graphical and batch mode
QCoreApplication* createQtApplication(int &argc, char *argv[], int gui)
{
	if(gui == 1)
		return new QCoreApplication(argc, argv);
	
	return nullptr;
//	return new QApplication(argc, argv);
}

