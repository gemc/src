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

    GOptions *gopts = new GOptions(argc, argv, GOptions());
    bool gui = gopts->getSwitch("gui");

    QScopedPointer <QCoreApplication> gApp(new QApplication(argc, argv));

    GSplash *gsplash = nullptr;
    Q_INIT_RESOURCE(qt_resources_example);

    if (gui) {

        gsplash = new GSplash("example.png");

        // wait 1 second
        QTime dieTime = QTime::currentTime().addSecs(1);
        while (QTime::currentTime() < dieTime) { QCoreApplication::processEvents(QEventLoop::AllEvents, 100); }
        gsplash->message("Some text I want to show");

        gApp->processEvents();

        QMainWindow window;
        window.show();
        gsplash->finish(&window);

        return gApp->exec();
    }

    return EXIT_SUCCESS;
}
