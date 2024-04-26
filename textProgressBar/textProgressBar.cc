// textProgressBar
#include "textProgressBar.h"

// c++
#include <iostream>
using std::cout, std::endl;

void TextProgressBar::setProgress(int p)
{
	// step too small
	if(p < indexStep) return;

	// step too small
	if(singleStep == 0) return;

	// outside boundaries
	if(p < min || p > max) return;

	// progress is between 1 and 100
	double progress = indexStep/singleStep;

	// progress outside boundaries
	if(progress > TEXTPROGRESSBARNSTEPS) return;

	// adding the single step
	indexStep += singleStep;

	// printing header of the bar
	cout  << title << startBarChar ;

	// pos is relative position within the bar
	int pos = barWidth*(progress/TEXTPROGRESSBARNSTEPS);

	// print bar, and advance char, empty space at the right places
	for (int i = 0; i < barWidth; ++i) {
		if      (i < pos)  cout << middleBarChar;
		else if (i == pos) cout << advanceBarChar;
		else cout << " ";
	}
	// print end bar and progress percentage
	cout << endBarChar << " " << progress << " %\r" ;

	// carriage return if needed
	if(progress == TEXTPROGRESSBARNSTEPS) cout << endl;

	cout.flush();
}
