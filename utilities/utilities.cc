// Qt
#include <QtWidgets>

// distinguishing between graphical and batch mode
QCoreApplication* createQtApplication(int &argc, char *argv[], bool gui)
{
	if(!gui)
		return new QCoreApplication(argc, argv);
	
	return nullptr;
//	return new QApplication(argc, argv);
}

