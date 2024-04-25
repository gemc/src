// gsplash
#include "gsplash.h"

// qt
#include <QApplication>

// c++
#include <iostream>
using namespace std;

// example of main declaring GOptions
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // if gui command line option is given - TODO: gui should be standard in goptions
    if (argc > 1) {
        // Iterate through command-line arguments
        for (int i = 1; i < argc; ++i) {
            // Compare the argument with 'gui'
            if (strcmp(argv[i], "gui") == 0) {
                // get image from env variable "GSPLASH"
                GSplash gsplash = GSplash("example");

                for(int i=0; i<100; i++) {
                    gsplash.message(to_string(i));
                }
                QMainWindow window;
                window.show();
                cout << "GUI option detected! TODO: gui should be standard in goptions" << endl;
                gsplash.finish(&window);
               // return app.exec(); // Exit the program
            }
        }
    }

	return EXIT_SUCCESS;
}
