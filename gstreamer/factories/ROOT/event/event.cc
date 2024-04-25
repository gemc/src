// gstreamer
#include "../gstreamerROOTFactory.h"

bool GstreamerRootFactory::startEvent(const GEventDataCollection* eventData)
{
	if(rootfile == nullptr) return false;

	// clearing maps vectors for all trees
	for(auto t: (*gRootTrees)) {
		t.second->initTreeForTheEvent();
	}

	return true;
}




bool GstreamerRootFactory::endEvent(const GEventDataCollection* eventData)
{
	if(rootfile == nullptr) return false;

	return true;
}
