/**
 * \file gframe_example.cc
 * \brief Example to test frame data features.
 *
 * \mainpage Frame Data Example
 *
 * \section intro_sec Introduction
 * This example demonstrates how to use the frame data classes to collect
 * integrated payloads. A frame header is created along with a frame data collection.
 * Several integral payloads are added and then printed.
 *
 * \section usage_sec Usage:
 * Compile this file along with the GFrameDataCollection, GFrameDataCollectionHeader,
 * GIntegralPayload, gdata_options, and glogger modules.
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n */

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


// TODO: run this in multiple threads and collect results in frames like in runAction
int main(int argc, char *argv[]) {
	// Create GOptions using gevent_data::defineOptions, which aggregates options from all gdata and gtouchable.
	auto gopts = std::make_shared<GOptions>(argc, argv, gevent_data::defineOptions());

	// Create loggers: one for gdata and one for gtouchable.
	auto log = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, GEVENTDATA_LOGGER);

	auto logt = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, TOUCHABLE_LOGGER);

	// Define a frame with a frame ID and frame duration.
	long int frameID = 1;
	double frameDuration = 33.33; // Example frame duration (units could be ms or other)

	// Create a frame header. The header logs its construction.
	auto frameHeader = new GFrameHeader(frameID, frameDuration, log);

	// Create the frame data collection using the header and logger.
	auto frameData = new GFrameDataCollection(frameHeader, log);

	// Create several integral payloads.
	// Each payload vector must contain exactly 5 integers: [crate, slot, channel, charge, time]
	vector<int> payload1 = {1, 2, 3, 100, 50};
	vector<int> payload2 = {4, 5, 6, 200, 75};
	vector<int> payload3 = {7, 8, 9, 150, 60};

	// Add the payloads to the frame data.
	frameData->addIntegralPayload(payload1);
	frameData->addIntegralPayload(payload2);
	frameData->addIntegralPayload(payload3);

	// Test getters: Print the frame ID and computed time from the header.
	cout << "Frame ID: " << frameData->getFrameID() << endl;
	cout << "Frame Header Time: " << frameData->getHeader()->getTime() << endl;

	// Retrieve the integral payloads vector.
	const vector<GIntegralPayload*> *payloads = frameData->getIntegralPayload();
	cout << "Number of integral payloads: " << payloads->size() << endl;

	// Print details of each integral payload.
	for (size_t i = 0; i < payloads->size(); ++i) {
		vector<int> p = (*payloads)[i]->getPayload();
		cout << "Payload " << (i + 1) << ": ";
		for (auto v : p) {
			cout << v << " ";
		}
		cout << endl;
	}

	// Cleanup: Delete frame data and options.
	delete frameData;  // This deletes the header and all integral payloads inside.

	return EXIT_SUCCESS;
}
