// gQtButtonsWidget
#include "gQtButtonsWidget.h"
#include "gQtButtonsWidgetOptions.h"

// c++
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {

    GOptions *gopts = new GOptions(argc, argv, gQtButtonsWidget::defineOptions());

    QApplication app(argc, argv);

    vector <string> bicons;

    // notice path must match the path in the qrc file
    bicons.push_back(":/images/firstButton");
    bicons.push_back(":/images/secondButton");

    if (gopts->getSwitch("gui")) {
        GQTButtonsWidget window(128, 128, bicons);
        window.show();
        return app.exec();
    }

    return EXIT_SUCCESS;
}
