#pragma once

// c++
#include <map>

// ROOT
#include "TTree.h"
#include "ROOT/TThreadedObject.hxx"

// gemc
#include "event/gEventDataCollection.h"
#include "run/gRunDataCollection.h"

#define EVENTHEADERTREENAME "event_header"
#define RUNHEADERTREENAME   "run_header"
#define TRUEINFONAMEPREFIX  "true_info_"
#define DIGITIZEDNAMEPREFIX "digitized_"

#define ERR_GSTREAMERROOTTREENOTFOUND 850

#define EVENTHEADERTREENAMEDESC "Event Header"
#define RUNHEADERTREENAMEDESC   "Run Header"
#define TRUEINFOTREENAMEDESC    "True Info Data"
#define DIGITIZEDTREENAMEDESC   "Digitized Data"

/**
 * \file gRootTree.h
 * \brief ROOT tree adapter used internally by the ROOT gstreamer plugin.
 * \ingroup gstreamer_plugin_root_api
 */

/**
 * \class GRootTree
 * \ingroup gstreamer_plugin_root_api
 * \brief Helper class encapsulating one ROOT \c TTree and its branch storage.
 *
 * This class adapts GEMC header and hit data models to ROOT vector branches. It owns:
 * - one \c TTree
 * - one set of branch storage maps keyed by variable name
 * - the logic needed to clear, refill, and write one entry per publish call
 *
 * Data organization:
 * - integer variables are stored in \c std::vector<int> branches
 * - floating-point variables are stored in \c std::vector<double> branches
 * - text variables are stored in \c std::vector<std::string> branches
 *
 * For hit-based trees, one branch vector entry corresponds to one hit in the published detector
 * collection. The same hit index therefore lines up across all branch vectors in a given tree fill.
 */
class GRootTree
{
public:
	/// \brief Destructor emitting a debug trace when a logger is available.
	~GRootTree() { if (log) log->debug(NORMAL, "~GRootTree"); }

	/**
	 * \brief Construct an event-header tree and register its branches.
	 *
	 * The event-header schema contains:
	 * - \c g4localEventNumber
	 * - \c threadID
	 * - \c timeStamp
	 *
	 * \param gevent_header Event header used to determine and initialize the schema.
	 * \param log Logger used for diagnostics.
	 */
	GRootTree([[maybe_unused]] const std::unique_ptr<GEventHeader>& gevent_header, std::shared_ptr<GLogger>& log);

	/**
	 * \brief Construct a run-header tree and register its branches.
	 *
	 * The run-header schema contains the run identifier.
	 *
	 * \param grun_header Run header used to determine and initialize the schema.
	 * \param log Logger used for diagnostics.
	 */
	GRootTree([[maybe_unused]] const std::unique_ptr<GRunHeader>& grun_header, std::shared_ptr<GLogger>& log);

	/**
	 * \brief Construct a true-information tree for one detector and register its branches.
	 *
	 * Branches are inferred from the variable maps exposed by the sample hit:
	 * - double-valued variables become \c std::vector<double> branches
	 * - string-valued variables become \c std::vector<std::string> branches
	 *
	 * \param detectorName Final ROOT tree name for this detector collection.
	 * \param gdata Sample true-information hit used to determine the schema.
	 * \param log Logger used for diagnostics.
	 */
	GRootTree(const std::string& detectorName, const GTrueInfoData* gdata, std::shared_ptr<GLogger>& log);

	/**
	 * \brief Construct a digitized-data tree for one detector and register its branches.
	 *
	 * Branches are inferred from the observable maps exposed by the sample hit:
	 * - integer observables become \c std::vector<int> branches
	 * - floating-point observables become \c std::vector<double> branches
	 *
	 * \param detectorName Final ROOT tree name for this detector collection.
	 * \param gdata Sample digitized hit used to determine the schema.
	 * \param log Logger used for diagnostics.
	 */
	GRootTree(const std::string& detectorName, const GDigitizedData* gdata, std::shared_ptr<GLogger>& log);

	/**
	 * \brief Fill the event-header tree with one event header entry.
	 *
	 * The method clears the branch storage vectors, inserts the current header values, and calls
	 * \c TTree::Fill().
	 *
	 * \param gevent_header Event header providing the values for this entry.
	 * \return \c true on success.
	 */
	bool fillTree(const std::unique_ptr<GEventHeader>& gevent_header);

	/**
	 * \brief Fill the run-header tree with one run header entry.
	 *
	 * \param run_header Run header providing the values for this entry.
	 * \return \c true on success.
	 */
	bool fillTree(const std::unique_ptr<GRunHeader>& run_header);

	/**
	 * \brief Fill a true-information detector tree with one detector collection.
	 *
	 * Each hit contributes one entry to each registered branch vector.
	 *
	 * \param trueInfoData Raw-pointer view of the detector true-information hits.
	 * \return \c true on success.
	 */
	bool fillTree(const std::vector<const GTrueInfoData*>& trueInfoData);

	/**
	 * \brief Fill a digitized detector tree with one detector collection.
	 *
	 * Each hit contributes one entry to each registered branch vector.
	 *
	 * \param digitizedData Raw-pointer view of the detector digitized hits.
	 * \return \c true on success.
	 */
	bool fillTree(const std::vector<const GDigitizedData*>& digitizedData);

private:
	/// \brief Owned ROOT tree instance receiving all branch data.
	std::unique_ptr<TTree> root_tree;

	/// \brief Integer branch storage keyed by variable name.
	std::map<std::string, std::vector<int>> intVarsMap;

	/// \brief Floating-point branch storage keyed by the variable name.
	std::map<std::string, std::vector<double>> doubleVarsMap;

	/// \brief String branch storage keyed by the variable name.
	std::map<std::string, std::vector<std::string>>stringVarsMap;

	/**
	 * \brief Register one integer branch.
	 *
	 * \param varname Branch name.
	 * \param value Sample value used only to select the overload.
	 */
	void registerVariable(const std::string& varname, int value, bool can_ignore_duplicates = false);

	/**
	 * \brief Register one floating-point branch.
	 *
	 * \param varname Branch name.
	 * \param value Sample value used only to select the overload.
	 */
	void registerVariable(const std::string& varname, double value);

	/**
	 * \brief Register one string branch.
	 *
	 * \param varname Branch name.
	 * \param value Sample value used only to select the overload.
	 */
	void registerVariable(const std::string& varname, const std::string& value);

	/// \brief Logger reference shared with the owning ROOT streamer plugin.
	std::shared_ptr<GLogger>& log;
};