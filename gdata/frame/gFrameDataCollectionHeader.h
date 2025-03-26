#ifndef  GFRAMEHEADER_H
#define  GFRAMEHEADER_H  1

// gdata
#include "../gdataConventions.h"

// gemc
#include "glogger.h"

// c++
#include <iostream>

class GFrameDataCollectionHeader {
public:
	GFrameDataCollectionHeader(long int frameID_, float frameDuration_, GLogger *const logger) : frameID(frameID_), frameDuration(frameDuration_), log(logger) {
		log->debug(CONSTRUCTOR, "GFrameHeader id ", frameID);
	}

	~GFrameDataCollectionHeader() {
		log->debug(DESTRUCTOR, "GFrameHeader id ", frameID);
	}

	// getters
	inline long int getFrameID() const { return frameID; }

	inline long int getTime() const { return time_ns(); }


private:

	GLogger *const log;

	long int frameID;
	float frameDuration;

	long int time_ns() const  {return frameID*frameDuration;}

};


#endif
