#pragma once

// gstreamer
#include "gstreamer.h"
#include "gRootTree.h"

// ROOT
#include "TFile.h"

/**
 * \class GstreamerRootFactory
 * \brief ROOT output gstreamer plugin writing event data into TTrees.
 *
 * Output model:
 * - A single ROOT file is created per streamer instance (typically one instance per thread).
 * - The plugin maintains a map of \c GRootTree objects:
 *   - one header tree (key \c HEADERTREENAME)
 *   - one true info tree per detector (name prefixed by \c TRUEINFONAMEPREFIX)
 *   - one digitized tree per detector (name prefixed by \c DIGITIZEDNAMEPREFIX)
 *
 * The plugin creates trees lazily on first use to match the variable set encountered in data.
 *
 * Threading:
 * - Intended usage is one plugin instance per thread (one output file per thread).
 * - ROOT thread safety is enabled in the plugin initialization code.
 */
class GstreamerRootFactory : public GStreamer
{
public:
	// inherit the base (const std::shared_ptr<GOptions>&) ctor
	using GStreamer::GStreamer;

private:
	// open and close the output media
	bool openConnection() override;
	bool closeConnectionImpl() override;

	// event streams
	bool startEventImpl([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) override;
	bool endEventImpl([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) override;

	bool publishEventHeaderImpl([[maybe_unused]] const std::unique_ptr<GEventHeader>& gevent_header) override;
	bool publishEventTrueInfoDataImpl(const std::string&                       detectorName,
	                                  const std::vector<const GTrueInfoData*>& trueInfoData) override;
	bool publishEventDigitizedDataImpl(const std::string&                        detectorName,
	                                   const std::vector<const GDigitizedData*>& digitizedData) override;

	// frame streams
	bool startStreamImpl(const GFrameDataCollection* frameRunData) override;
	bool endStreamImpl(const GFrameDataCollection* frameRunData) override;
	bool publishFrameHeaderImpl(const GFrameHeader* gframeHeader) override;
	bool publishPayloadImpl(const std::vector<GIntegralPayload*>* payload) override;

	// returning raw pointers for access
	// Note: these are private helpers; do not cross-reference them with \ref in documentation.
	const std::unique_ptr<GRootTree>& getOrInstantiateHeaderTree(
		[[maybe_unused]] const std::unique_ptr<GEventHeader>& event_header);
	const std::unique_ptr<GRootTree>& getOrInstantiateTrueInfoDataTree(const std::string&   treeName,
	                                                                   const GTrueInfoData* gdata);
	const std::unique_ptr<GRootTree>& getOrInstantiateDigitizedDataTree(const std::string&    treeName,
	                                                                    const GDigitizedData* gdata);

	/// \brief Key is detector prefix and name for true/digitized trees, or \c "event_header" for the header tree.
	std::unordered_map<std::string, std::unique_ptr<GRootTree>> gRootTrees;

	/// \brief ROOT file owning the TTrees for this plugin instance.
	std::unique_ptr<TFile> rootfile; // ROOT file pointer

	/// \brief Return the output filename for this instance (\c ".root").
	[[nodiscard]] std::string filename() const override { return gstreamer_definitions.rootname + ".root"; }
};
