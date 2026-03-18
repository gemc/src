/**
 * \file gframe_example.cc
 * \anchor gframe_example
 * \brief Example demonstrating frame data collection.
 *
 * \details
 * Summary:
 * - creates one frame header and one frame data collection
 * - inserts three packed integral payloads
 * - prints the stored frame metadata and payload content
 *
 * Link to the example overview:
 * - \ref gframe_example
 */

/**
 * \defgroup gdata_example_frame_collection Frame data example
 * \brief Frame/time-window grouping of integrated readout payloads.
 *
 * \details
 * This example demonstrates how to build a frame container, GFrameDataCollection, that owns:
 * - one GFrameHeader containing frame ID and duration
 * - a list of GIntegralPayload objects storing crate, slot, channel, charge, and time
 *
 * Frames are typically used for streaming/readout-style output where data are grouped by
 * time windows rather than by events. Conceptually, a frame corresponds to a fixed integration
 * window during which many channels may contribute payload.
 */

/** @ingroup gdata_example_frame_collection
 * \section gdata_frame_payload_layout Payload layout
 * The \ref GFrameDataCollection::addIntegralPayload "addIntegralPayload()" API accepts a packed
 * integer vector with a fixed order and exact size 5:
 * - payload[0] crate
 * - payload[1] slot
 * - payload[2] channel
 * - payload[3] charge
 * - payload[4] time
 *
 * This example constructs three such payloads and inserts them into the frame collection.
 *
 * \section gdata_frame_usage Usage
 * Compile this file together with the frame classes and the logging/options utilities. Run it to:
 * - print the frame ID and computed frame time
 * - print the number of stored payloads
 * - print each payload in its packed fixed order
 *
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 *
 * @{
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
#include <string>
#include <vector>

using namespace std;

// TODO: Run this in multiple threads and collect results into frames, similar to a runAction-style aggregator.

/**
 * \brief Entry point for the frame example.
 *
 * \details
 * The program:
 * - constructs the option bundle used by the example
 * - creates loggers
 * - creates one frame header and one frame container
 * - inserts three example packed payload vectors
 * - prints the frame metadata and each stored payload
 * - deletes the frame container, which also deletes the owned header and payload objects
 *
 * This example intentionally uses the raw-pointer ownership pattern currently implemented by
 * GFrameDataCollection so the generated documentation matches the actual API contract.
 *
 * \param argc Argument count forwarded to GOptions.
 * \param argv Argument vector forwarded to GOptions.
 * \return \c EXIT_SUCCESS on normal completion.
 */
int main(int argc, char* argv[]) {
	// Build the option bundle used by this example.
	auto gopts = std::make_shared<GOptions>(argc, argv, gevent_data::defineOptions());

	// Create loggers used by the frame objects and by any touchable-related code pulled in by the options bundle.
	auto log  = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, GEVENTDATA_LOGGER);
	auto logt = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, TOUCHABLE_LOGGER);

	long int frameID       = 1;
	double   frameDuration = 33.33; // Example frame duration, with units defined by the caller.

	// Create the frame header and transfer ownership into the frame collection.
	auto frameHeader = new GFrameHeader(frameID, frameDuration, log);
	auto frameData   = new GFrameDataCollection(frameHeader, log);

	// Build three packed payload vectors matching the fixed crate/slot/channel/charge/time layout.
	vector<int> payload1 = {1, 2, 3, 100, 50};
	vector<int> payload2 = {4, 5, 6, 200, 75};
	vector<int> payload3 = {7, 8, 9, 150, 60};

	// Insert the payloads into the frame collection.
	frameData->addIntegralPayload(payload1);
	frameData->addIntegralPayload(payload2);
	frameData->addIntegralPayload(payload3);

	cout << "Frame ID: " << frameData->getFrameID() << endl;
	cout << "Frame Header Time: " << frameData->getHeader()->getTime() << endl;

	const vector<GIntegralPayload*>* payloads = frameData->getIntegralPayload();
	cout << "Number of integral payloads: " << payloads->size() << endl;

	// Print each stored payload in its fixed exported order.
	for (size_t i = 0; i < payloads->size(); ++i) {
		vector<int> p = (*payloads)[i]->getPayload();
		cout << "Payload " << (i + 1) << ": ";
		for (auto v : p) {
			cout << v << " ";
		}
		cout << endl;
	}

	delete frameData; // Deletes the header and all payloads owned by the collection.

	return EXIT_SUCCESS;
}

/** @} */