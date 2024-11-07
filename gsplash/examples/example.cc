// gsplash
#include "gsplash.h"
#include "goptions.h"

// qt
#include <QApplication>

// c++
#include <iostream>

using namespace std;

// no idea why this is not working
int main(int argc, char *argv[]) {
    GOptions empty_options;

    GOptions *gopts = new GOptions(argc, argv, empty_options);

    bool gui = gopts->getSwitch("gui");

    GSplash *gsplash = nullptr;
    QApplication *gApp = new QApplication(argc, argv);

    if (gui) {

        gsplash = new GSplash("splash_example");

        // wait 1 second
        QTime dieTime = QTime::currentTime().addSecs(1);
        while (QTime::currentTime() < dieTime) { QCoreApplication::processEvents(QEventLoop::AllEvents, 100); }

        gApp->processEvents();

        QMainWindow window;
        window.show();
        gsplash->finish(&window);

        return gApp->exec(); // Exit the program
    }


    return EXIT_SUCCESS;
}
