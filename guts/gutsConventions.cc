// conventions
#include "gutsConventions.h"

// thread safe message systems

#include <mutex>
std::mutex mu;

// c++
#include <iostream>
using namespace std;

// exiting with error, print error on screen.
void gexit(int error) {
	mu.lock();
	if ( error != EXIT_SUCCESS ) {
		cerr << KBOLD << KRED << " Exiting with error " << error << RST << endl;
	}
	mu.unlock();
	exit(error);
}

// class constructor
void gLogClassConstruct(string className) {
	mu.lock();
	cout << KGRN << CONSTRUCTORLOG <<  className << KWHT << " class constructor" << RST << endl;
	mu.unlock();
}

// copy constructor
void gLogCopyConstruct(string className) {
	mu.lock();
	cout << KGRN << CONSTRUCTORLOG <<  className << KWHT << " copy constructor" << RST << endl;
	mu.unlock();
}

// move constructor
void gLogMoveConstruct(string className) {
	mu.lock();
	cout << KBLU << CONSTRUCTORLOG <<  className << KWHT << " move constructor" << RST << endl;
	mu.unlock();
}

// destructor
void gLogClassDestruct(string className) {
	mu.lock();
	cout << KRED << DESTRUCTORLOG <<  className << KWHT << " destructor" << RST << endl;
	mu.unlock();
}

void gLogMessage(string message) {
	mu.lock();
	cout <<  message << endl;
	mu.unlock();
}
