// gQtButtonsWidget
#include "gQtButtonsWidget.h"

// c++
#include <iostream>
using namespace std;


int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QFileInfo qrcFileInfoExecutable(argv[0]);
    QString rccPath = qrcFileInfoExecutable.absolutePath() + "/" + "exampleResources.rcc";
    QResource::registerResource(rccPath);

    vector <string> bicons;

    // notice path must match the path in the qrc file
    bicons.push_back(":/images/firstButton");
    bicons.push_back(":/images/secondButton");

    GQTButtonsWidget window(128, 128, bicons);
    window.show();

    // if gui command line option is given - TODO: gui should be standard in goptions
    if (argc > 1) {
        // Iterate through command-line arguments
        for (int i = 1; i < argc; ++i) {
            // Compare the argument with 'gui'
            if (strcmp(argv[i], "gui") == 0) {
                cout << "GUI option detected! TODO: gui should be standard in goptions" << endl;
                return app.exec(); // Exit the program
            }
        }
    }



    return EXIT_SUCCESS;
}
