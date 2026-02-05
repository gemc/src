#pragma once

// geant4
#include "G4UserRunAction.hh"


// gemc
#include "gbase.h"
#include "gdynamicdigitization.h"
#include "gstreamer.h"
// #include "frame/gFrameDataCollection.h"

/**
 * @file gRunAction.h
 * @brief Declares GRunAction, responsible for run lifecycle hooks and run object creation.
 *
 * @ingroup gactions_module
 */

constexpr const char* GRUNACTION_LOGGER = "grunaction";

/**
 * @brief Namespace collecting helpers for the run action.
 *
 * @ingroup gactions_module
 */
namespace grunaction {
/**
 * @brief Returns the options associated with the run action.
 *
 * @return A GOptions instance for the run action logger scope.
 */
inline GOptions defineOptions() { return GOptions(GRUNACTION_LOGGER); }
} // namespace grunaction


/**
 * @class GRunAction
 * @brief Handles run begin/end callbacks and creates the per-thread run object.
 *
 * Responsibilities:
 * - Create the per-thread run object (GRun) via GenerateRun().
 * - At the start of each run on worker threads, create the per-thread streamer map
 *   (once) and open streamer connections for the run.
 * - At the end of each run on worker threads, close streamer connections.
 *
 * Threading:
 * - Worker threads own their local streamer map instance.
 * - The master thread typically does not create streamers and primarily exists to
 *   coordinate the run lifecycle.
 *
 * @ingroup gactions_module
 */
class GRunAction : public GBase<GRunAction>, public G4UserRunAction {
public:
	/**
	 * @brief Constructs the run action.
	 *
	 * @param gopts Shared configuration used by the run action and by the created run object.
	 * @param digi_map Shared digitization routines map to be passed into the created run object.
	 */
	GRunAction(std::shared_ptr<GOptions> gopts, std::shared_ptr<gdynamicdigitization::dRoutinesMap> digi_map);

	/**
	 * @brief Returns the shared digitization routines map.
	 *
	 * This map is used by event-level logic to select the correct digitization routine
	 * for each hit collection.
	 *
	 * @return Shared pointer to the digitization routines map.
	 */
	auto get_digitization_routines_map() const -> std::shared_ptr<gdynamicdigitization::dRoutinesMap>  {
		return digitization_routines_map;
	}

	/**
	 * @brief Returns the per-thread streamer map, if it has been created.
	 *
	 * The map is instantiated lazily for worker threads at the beginning of a run.
	 *
	 * @return Shared pointer to the streamer map (may be nullptr if not yet created or on master).
	 */
	auto get_streamer_map() const -> std::shared_ptr<const gstreamer::gstreamersMap> {
		return gstreamer_map;
	}

private:
	/**
	 * @brief Creates and returns the run object for the current thread.
	 *
	 * This is invoked by Geant4 after BeamOn() begins and is used to create the
	 * thread-local GRun instance.
	 *
	 * @return Newly allocated run object for the current thread.
	 */
	G4Run* GenerateRun() override;

	/**
	 * @brief Called by Geant4 at the beginning of a run.
	 *
	 * Worker-thread behavior:
	 * - Lazily instantiate the streamer map (once).
	 * - Open each streamer connection for the run.
	 *
	 * @param run The Geant4 run descriptor for the current run.
	 */
	void BeginOfRunAction(const G4Run* run) override;

	/**
	 * @brief Called by Geant4 at the end of a run.
	 *
	 * Worker-thread behavior:
	 * - Close each streamer connection for the run.
	 *
	 * @param run The Geant4 run descriptor for the current run.
	 */
	void EndOfRunAction(const G4Run* run) override;

	/**
	 * @brief Shared configuration used by this run action and passed to dependent components.
	 */
	std::shared_ptr<GOptions> goptions;

	/**
	 * @brief Digitization routines map used by run/event actions to digitize hit collections.
	 */
	std::shared_ptr<gdynamicdigitization::dRoutinesMap> digitization_routines_map;

	/**
	 * @brief Per-thread streamer map (worker threads only), instantiated at run start.
	 */
	std::shared_ptr<const gstreamer::gstreamersMap> gstreamer_map;
};



// vector of frame data in the run (local thread, merged in GRun::Merge in the global thread)
//	std::vector<GFrameDataCollection*> frameRunData;

//
// double frameDuration    = UNINITIALIZEDNUMBERQUANTITY; // frame length in nanoseconds
// double eventDuration    = UNINITIALIZEDNUMBERQUANTITY; // event duration in nanoseconds
// int    eventIndex       = 0;                           // added to the absolute event number, increases with each run
// int    lastFrameCreated = 0;                           // keeping track of the last frame created
//
// // determine the frame ID based on event number, eventDuration, frameDuration
// // TODO: this should be in gdata?
// int  eventFrameIndex(int eventNumber, double timeAtElectronics);
// bool findFrameID(int fid);
//
// // decide whether to write or not to stream the frame based on event number, eventDuration, frameDuration and number of threads
// // streaming the frame also deletes it from frameRunData
// // TODO: this should be in gdata?
// bool shouldWriteFrameID(int eventNumber, long int frameID);
//
// // TODO: this should be in gdata?
// std::vector<int> formPayload(GDigitizedData* digitizedData);


// in the constructur we had:

// frameDuration = 64000;
// eventDuration = gutilities::getG4Number(goptions->getScalarString("eventTimeSize"));

// stream = gopt->getSwitch("stream");
