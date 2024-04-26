/// \file example.cc

// example on how to use the gdynamic library

// gdynamic
#include "gdynamicdigitization.h"

// gfactory
#include "gfactory.h"

// c++
#include <iostream>

using namespace std;


//! example of main declaring GOptions
//int main(int argc, char *argv[]) {
int main() {

    GManager manager("example", 10);

    // using dynamicRoutines map
    map < string, GDynamicDigitization * > dynamicRoutines;
    dynamicRoutines["ctof"] = manager.LoadAndRegisterObjectFromLibrary<GDynamicDigitization>("CTofRoutineExample");

    // using shared_ptr
    shared_ptr <GDynamicDigitization> globalCtof1(manager.LoadAndRegisterObjectFromLibrary<GDynamicDigitization>("CTofRoutineExample"));
    shared_ptr <GDynamicDigitization> globalCtof2(manager.LoadAndRegisterObjectFromLibrary<GDynamicDigitization>("CTofRoutineExample"));

    // increments reference count
    // to be used in the local thread
    auto globalCtof3(globalCtof2);

    cout << dynamicRoutines["ctof"] << " " << globalCtof1 << " " << globalCtof2 << " " << globalCtof3 << endl;

    dynamicRoutines["ctof"]->loadConstants(1, "original");

    globalCtof1->loadConstants(1, "original");
    globalCtof2->loadConstants(1, "original");
    globalCtof3->loadConstants(1, "original");

    return EXIT_SUCCESS;
}
