// textProgressBar
#include "textProgressBar.h"

// c++
#include <iostream>
using namespace std;


int main(int argc, char* argv[])
{

	int nevents = 100000000;

	TextProgressBar bar(30, "  index progress: ", 1, nevents);

	for(int i=1; i<nevents; i++) {
		bar.setProgress(i);
	}


	return EXIT_SUCCESS;
}
