/**
 * \file gframe_example.cc
 * \brief Example demonstrating frame data collection.
 *
 * \page gdata_frame_example Frame data example
 *
 * \section frame_overview Overview
 * This example demonstrates how to build a frame container (\ref GFrameDataCollection) that owns:
 * - a \ref GFrameHeader (frame ID + duration)
 * - a list of \ref GIntegralPayload objects (crate/slot/channel/charge/time)
 *
 * Frames are typically used for streaming/readout-style output where data are grouped by
 * time windows rather than by Geant4 events. Conceptually, a frame corresponds to a fixed
 * integration window (for example 33.33 ms), during which many channels may fire.
 *
 * \section frame_payload_layout Payload layout
 * The \ref GFrameDataCollection::addIntegralPayload "addIntegralPayload()" API accepts a packed
 * integer vector with a fixed order (size must be exactly 5):
 * - payload[0] crate
 * - payload[1] slot
 * - payload[2] channel
 * - payload[3] charge
 * - payload[4] time
 *
 * This example constructs three such payloads and inserts them into the frame collection.
 *
 * \section gdata_frame_usage Usage
 * Compile this file along with the frame classes and the logging/options utilities. Run it to:
 * - print the frame ID and computed frame time
 * - print each stored payload in its fixed order
 *
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 */

// gdata
#include "frame/gFrameDataCollection.h"
#include "frame/gFrameHeader.h"
#include "gEventDataCollection.h"

// gemc
#include "glogger.h"
#include <gtouchable_options.h>

// c++
#include <iostream>
#include <vector>
#include <string>

using namespace std;

// TODO: Run this in multiple threads and collect results into frames (similar to a runAction-style aggregator).
int main(int argc, char* argv[]) {
	auto gopts = std::make_shared<GOptions>(argc, argv, gevent_data::defineOptions());

	auto log  = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, GEVENTDATA_LOGGER);
	auto logt = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, TOUCHABLE_LOGGER);

	long int frameID       = 1;
	double   frameDuration = 33.33; // Example frame duration (units could be ms or other)

	auto frameHeader = new GFrameHeader(frameID, frameDuration, log);
	auto frameData   = new GFrameDataCollection(frameHeader, log);

	vector<int> payload1 = {1, 2, 3, 100, 50};
	vector<int> payload2 = {4, 5, 6, 200, 75};
	vector<int> payload3 = {7, 8, 9, 150, 60};

	frameData->addIntegralPayload(payload1);
	frameData->addIntegralPayload(payload2);
	frameData->addIntegralPayload(payload3);

	cout << "Frame ID: " << frameData->getFrameID() << endl;
	cout << "Frame Header Time: " << frameData->getHeader()->getTime() << endl;

	const vector<GIntegralPayload*>* payloads = frameData->getIntegralPayload();
	cout << "Number of integral payloads: " << payloads->size() << endl;

	for (size_t i = 0; i < payloads->size(); ++i) {
		vector<int> p = (*payloads)[i]->getPayload();
		cout << "Payload " << (i + 1) << ": ";
		for (auto v : p) {
			cout << v << " ";
		}
		cout << endl;
	}

	delete frameData; // deletes header and all payloads inside

	return EXIT_SUCCESS;
}
