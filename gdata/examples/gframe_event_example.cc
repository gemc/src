/**
 * \file frame_data_example.cpp
 * \brief Example to test frame data features.
 *
 * \mainpage Frame Data Example
 *
 * \section intro_sec Introduction
 * This example demonstrates how to use the frame data classes to collect
 * integrated payloads. A frame header is created along with a frame data collection.
 * Several integral payloads are added and then printed.
 *
 * \section usage_sec Usage
 * Compile this file along with the GFrameDataCollection, GFrameDataCollectionHeader,
 * GIntegralPayload, gdata_options, and glogger modules.
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n */

// gdata
#include "frame/gFrameDataCollection.h"
#include "frame/gFrameDataCollectionHeader.h"
#include "gdata_options.h"

// gemc
#include "glogger.h"

// c++
#include <iostream>
#include <vector>
#include <string>

using namespace std;

int main(int argc, char *argv[]) {
	// Create GOptions using gdata's defineOptions.
	GOptions *gopts = new GOptions(argc, argv, gdata::defineOptions());

	// Create a logger for the gdata module.
	GLogger *log = new GLogger(gopts, DATA_LOGGER);

	// Define a frame with a frame ID and frame duration.
	long int frameID = 1;
	float frameDuration = 33.33; // Example frame duration (units could be ms or other)

	// Create a frame header. The header logs its construction.
	GFrameDataCollectionHeader *frameHeader = new GFrameDataCollectionHeader(frameID, frameDuration, log);

	// Create the frame data collection using the header and logger.
	GFrameDataCollection *frameData = new GFrameDataCollection(frameHeader, log);

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
	delete gopts;      // Logger deletion is handled by frame data and others as needed.

	return EXIT_SUCCESS;
}
