// gstreamer
#include "../gstreamerTEXTFactory.h"


bool GstreamerTextFactory::publishEventHeader(const GEventDataCollectionHeader *gheader)
{
	if(ofile == nullptr) return false;
	
	*ofile << GTAB << "Header Bank {" << endl;
	*ofile << GTABTAB << " time: " << gheader->getTimeStamp() << endl;
	*ofile << GTABTAB << " g4run-local event number: " << gheader->getG4LocalEvn() << endl;
	*ofile << GTABTAB << " thread id: " << gheader->getThreadID() << endl;
//	*ofile << GTABTAB << " random status: " << gh->getG4Rnd() << endl;
	*ofile << GTAB << "}" << endl;

	return true;
}
