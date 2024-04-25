#ifndef  TEXTPROGRESSBAR_H
#define  TEXTPROGRESSBAR_H  1

#include "textProgressBarConventions.h"

// c++
#include <string>
using std::string;

class TextProgressBar
{

public:
	TextProgressBar(int w, string t, int minimum = 0, int maximum = 100, char s='[', char e=']', char m='=', char a='>') :
	barWidth(w), title(t), min(minimum), max(maximum), startBarChar(s), endBarChar(e), middleBarChar(m), advanceBarChar(a) {

		if(maximum - minimum > TEXTPROGRESSBARNSTEPS) {
			singleStep  = (maximum - minimum) / TEXTPROGRESSBARNSTEPS ;
		} else {
			singleStep = 0;
		}
		indexStep = minimum;
	}

	void setProgress(int p);

private:
	int barWidth;
	string title;
	int min, max;
	char startBarChar, endBarChar;
	char middleBarChar, advanceBarChar;

	int singleStep, indexStep;


};


#endif
