// gutilities
#include "gutilities.h"
#include "gutsConventions.h"

// c++
#include <iostream>
using namespace std;


int main(int argc, char *argv[]) {

    if (argc != 2) {
        cerr << FATALERRORL << "run example with exactly one argument: a quotes enclosed list of strings." << endl;
        return EXIT_FAILURE;

    } else {
        string test = argv[1];
        vector <string> testResult = gutilities::getStringVectorFromString(test);

        cout << " Original String: " << test << endl;
        cout << " List of items in string: " << endl;
        for (auto &s: testResult) {
            cout << SQUAREITEM << s << endl;
        }
    }

    return EXIT_SUCCESS;
}
