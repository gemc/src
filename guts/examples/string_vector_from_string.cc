// gutilities
#include "gutilities.h"

using namespace gutilities;


// c++
#include <iostream>

using namespace std;


int main(int argc, char *argv[]) {

    if (argc != 2) {
        cerr << FATALERRORL << "run example with exactly one argument: a quotes enclosed list of strings." << endl;
    } else {

        string test = argv[1];
        vector <string> testResult = getStringVectorFromString(test);

        cout << " Original String: " << test << endl;
        cout << " List of items in string: " << endl;
        for (auto &s: testResult) {
            cout << SQUAREITEM << s << endl;
        }
    }

    return EXIT_SUCCESS;
}
