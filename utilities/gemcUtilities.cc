// Qt
#include <QtWidgets>

// distinguishing between graphical and batch mode
QCoreApplication* createQtApplication(int &argc, char *argv[], bool gui)
{
	if( !gui ) {
		return new QCoreApplication(argc, argv);
	} else {
		return new QApplication(argc, argv);
	}
}

