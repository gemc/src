// gdata 
#include "gEventDataCollectionHeader.h"

// c time
#include "ctime"

string GEventDataCollectionHeader::assignTimeStamp()
{
	time_t now = time(NULL);
	struct tm * ptm = localtime(&now);
	char buffer[32];

	// Format: Mo, 15.06.2009 20:20:00
	strftime (buffer, 32, "%a, %m.%d.%Y %H:%M:%S", ptm);

	return string(buffer);
}

