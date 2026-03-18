#pragma once

// gstreamer
#include "gstreamer.h"
#include "gRootTree.h"

// ROOT
#include "TFile.h"

/**
 * \file gstreamerROOTFactory.h
 * \brief ROOT streamer plugin declarations.
 * \ingroup gstreamer_plugin_root_api
 */

/**
 * \class GstreamerRootFactory
 * \ingroup gstreamer_plugin_root_api
 * \brief ROOT plugin writing event and run content into \c TTree objects stored in one \c TFile.
 *
 * Output model:
 * - one ROOT file is produced per plugin instance
 * - one header tree is created for event headers
 * - one header tree is created for run headers
 * - one true-information tree is created lazily per detector
 * - one digitized tree is created lazily per detector
 *
 * Tree creation is demand-driven. The first hit seen for a detector determines the variable schema
 * used to build the corresponding \c TTree via GRootTree.
 *
 * Threading model:
 * - one plugin instance per worker thread is the intended usage
 * - the plugin enables ROOT thread safety at library load time
 */
class GstreamerRootFactory : public GStreamer
{
public:
	/// \brief Inherit the constructor taking the parsed options container.
	using GStreamer::GStreamer;

private:
	/**
	 * \brief Open the ROOT output file for this plugin instance.
	 *
	 * \return \c true on success, \c false otherwise.
	 */
	bool openConnection() override;

	/**
	 * \brief Close the ROOT output file after writing pending in-memory content.
	 *
	 * \return \c true on success, \c false otherwise.
	 */
	bool closeConnectionImpl() override;

	/**
	 * \brief Begin one event publication cycle.
	 *
	 * \param event_data Event collection being published.
	 * \return \c true on success, \c false otherwise.
	 */
	bool startEventImpl([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) override;

	/**
	 * \brief End one event publication cycle.
	 *
	 * \param event_data Event collection being published.
	 * \return \c true on success, \c false otherwise.
	 */
	bool endEventImpl([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) override;

	/**
	 * \brief Publish one event header into the event-header ROOT tree.
	 *
	 * \param gevent_header Event header to serialize.
	 * \return \c true on success, \c false otherwise.
	 */
	bool publishEventHeaderImpl([[maybe_unused]] const std::unique_ptr<GEventHeader>& gevent_header) override;

	/**
	 * \brief Publish one detector true-information collection into its ROOT tree.
	 *
	 * \param detectorName Detector name used to derive the tree name.
	 * \param trueInfoData True-information hits for that detector.
	 * \return \c true on success, \c false otherwise.
	 */
	bool publishEventTrueInfoDataImpl(const std::string& detectorName,
	                                  const std::vector<const GTrueInfoData*>& trueInfoData) override;

	/**
	 * \brief Publish one detector digitized collection into its ROOT tree.
	 *
	 * \param detectorName Detector name used to derive the tree name.
	 * \param digitizedData Digitized hits for that detector.
	 * \return \c true on success, \c false otherwise.
	 */
	bool publishEventDigitizedDataImpl(const std::string& detectorName,
	                                   const std::vector<const GDigitizedData*>& digitizedData) override;

	/**
	 * \brief Begin one run publication cycle.
	 *
	 * \param run_data Run collection being published.
	 * \return \c true on success, \c false otherwise.
	 */
	bool startRunImpl([[maybe_unused]] const std::shared_ptr<GRunDataCollection>& run_data) override;

	/**
	 * \brief End one run publication cycle.
	 *
	 * \param run_data Run collection being published.
	 * \return \c true on success, \c false otherwise.
	 */
	bool endRunImpl([[maybe_unused]] const std::shared_ptr<GRunDataCollection>& run_data) override;

	/**
	 * \brief Publish one run header into the run-header ROOT tree.
	 *
	 * \param run_header Run header to serialize.
	 * \return \c true on success, \c false otherwise.
	 */
	bool publishRunHeaderImpl([[maybe_unused]] const std::unique_ptr<GRunHeader>& run_header) override;

	/**
	 * \brief Publish one detector run-level digitized collection into its ROOT tree.
	 *
	 * \param detectorName Detector name used to derive the tree name.
	 * \param digitizedData Run-level digitized data for that detector.
	 * \return \c true on success, \c false otherwise.
	 */
	bool publishRunDigitizedDataImpl(const std::string& detectorName,
									 const std::vector<const GDigitizedData*>& digitizedData) override;

	/**
	 * \brief Begin one frame-stream publication cycle.
	 *
	 * \param frameRunData Frame collection associated with the request.
	 * \return \c true on success, \c false otherwise.
	 */
	bool startStreamImpl(const GFrameDataCollection* frameRunData) override;

	/**
	 * \brief End one frame-stream publication cycle.
	 *
	 * \param frameRunData Frame collection associated with the request.
	 * \return \c true on success, \c false otherwise.
	 */
	bool endStreamImpl(const GFrameDataCollection* frameRunData) override;

	/**
	 * \brief Accept one frame header publish request.
	 *
	 * \param gframeHeader Frame header associated with the request.
	 * \return \c true on success, \c false otherwise.
	 */
	bool publishFrameHeaderImpl(const GFrameHeader* gframeHeader) override;

	/**
	 * \brief Accept one frame payload publish request.
	 *
	 * \param payload Frame payload associated with the request.
	 * \return \c true on success, \c false otherwise.
	 */
	bool publishPayloadImpl(const std::vector<GIntegralPayload*>* payload) override;

	// Private helper methods used to lazily create trees on first use.
	const std::unique_ptr<GRootTree>& getOrInstantiateHeaderTree(
		[[maybe_unused]] const std::unique_ptr<GEventHeader>& event_header);
	const std::unique_ptr<GRootTree>& getOrInstantiateHeaderTree(
		[[maybe_unused]] const std::unique_ptr<GRunHeader>& run_header);
	const std::unique_ptr<GRootTree>& getOrInstantiateTrueInfoDataTree(const std::string& treeName,
	                                                                   const GTrueInfoData* gdata);
	const std::unique_ptr<GRootTree>& getOrInstantiateDigitizedDataTree(const std::string& treeName,
	                                                                    const GDigitizedData* gdata);

	/// \brief Map of lazily created ROOT trees keyed by logical tree name.
	std::unordered_map<std::string, std::unique_ptr<GRootTree>> gRootTrees;

	/// \brief ROOT file owning all trees written by this plugin instance.
	std::unique_ptr<TFile> rootfile;

	/**
	 * \brief Return the final ROOT filename for this plugin instance.
	 *
	 * \return Base output name plus the \c ".root" extension.
	 */
	[[nodiscard]] std::string filename() const override { return gstreamer_definitions.rootname + ".root"; }
};