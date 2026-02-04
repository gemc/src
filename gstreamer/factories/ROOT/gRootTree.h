#pragma once

// c++
#include <map>

// ROOT
#include "TTree.h"
#include "ROOT/TThreadedObject.hxx"

// gemc
#include "event/gEventDataCollection.h"

#include "gstreamerConventions.h"

#define HEADERTREENAME "event_header"
#define TRUEINFONAMEPREFIX   "true_info_"
#define DIGITIZEDNAMEPREFIX  "digitized_"

#define ERR_GSTREAMERROOTTREENOTFOUND   850

#define HEADERTREENAMEDESC    "Event Header"
#define TRUEINFOTREENAMEDESC  "True Info Data"
#define DIGITIZEDTREENAMEDESC "Digitized Data"

/**
 * \file gRootTree.h
 * \brief ROOT tree adapter used by the ROOT gstreamer plugin.
 */

/**
 * \class GRootTree
 * \brief Adapter that owns a ROOT TTree and provides a type-safe fill interface for gstreamer data.
 *
 * This class encapsulates:
 * - creation of a TTree with a specific schema (header, true info, or digitized)
 * - branch registration based on variable maps encountered in the first data sample
 * - per-fill clearing and repopulation of branch vectors, then a call to \c Fill()
 *
 * Data model:
 * - Each branch is a \c std::vector<T> where T is \c int, \c double, or \c std::string.
 * - For hit banks, each vector stores one entry per hit; the vector index corresponds to hit index.
 *
 * Notes:
 * - This class is used internally by the ROOT plugin and is typically stored in a map keyed by tree name.
 * - Logging uses the module logger passed at construction time.
 */
class GRootTree
{
public:
	/// \brief Destructor prints a debug trace when a logger is present.
	~GRootTree() { if (log) log->debug(NORMAL, "~GRootTree"); }

	/**
	 * \brief Construct a header tree and register header branches.
	 *
	 * Branches registered:
	 * - \c g4localEventNumber
	 * - \c threadID
	 * - \c timeStamp
	 *
	 * \param gevent_header Event header providing initial values and schema.
	 * \param log Logger used for diagnostics.
	 */
	GRootTree([[maybe_unused]] const std::unique_ptr<GEventHeader>& gevent_header, std::shared_ptr<GLogger>& log);

	/**
	 * \brief Construct a true info tree and register branches from the provided hit.
	 *
	 * The schema is determined from the variable maps exposed by \p gdata:
	 * - double variables become \c std::vector<double> branches
	 * - string variables become \c std::vector<std::string> branches
	 *
	 * \param detectorName Tree name (already includes the true info prefix).
	 * \param gdata Sample true info hit used to determine the schema.
	 * \param log Logger used for diagnostics.
	 */
	GRootTree(const std::string& detectorName, const GTrueInfoData* gdata, std::shared_ptr<GLogger>& log);

	/**
	 * \brief Construct a digitized tree and register branches from the provided hit.
	 *
	 * The schema is determined from the observable maps exposed by \p gdata:
	 * - int observables become \c std::vector<int> branches
	 * - double observables become \c std::vector<double> branches
	 *
	 * \param detectorName Tree name (already includes the digitized prefix).
	 * \param gdata Sample digitized hit used to determine the schema.
	 * \param log Logger used for diagnostics.
	 */
	GRootTree(const std::string& detectorName, const GDigitizedData* gdata, std::shared_ptr<GLogger>& log);

	/**
	 * \brief Fill the header tree for one event.
	 *
	 * This clears previous header vectors, pushes the current values, then calls \c Fill().
	 *
	 * \param gevent_header Event header providing values for this entry.
	 * \return \c true on success.
	 */
	bool fillTree(const std::unique_ptr<GEventHeader>& gevent_header);

	/**
	 * \brief Fill the true info tree for one event and one detector.
	 *
	 * For each hit:
	 * - appends double variables to the corresponding double branch vectors
	 * - appends string variables to the corresponding string branch vectors
	 *
	 * \param trueInfoData Vector of raw pointers to true info hits.
	 * \return \c true on success.
	 */
	bool fillTree(const std::vector<const GTrueInfoData*>& trueInfoData);

	/**
	 * \brief Fill the digitized tree for one event and one detector.
	 *
	 * For each hit:
	 * - appends int observables to the corresponding int branch vectors
	 * - appends double observables to the corresponding double branch vectors
	 *
	 * \param digitizedData Vector of raw pointers to digitized hits.
	 * \return \c true on success.
	 */
	bool fillTree(const std::vector<const GDigitizedData*>& digitizedData);

private:
	/// \brief The owned ROOT tree instance.
	std::unique_ptr<TTree> root_tree;

	// variable maps
	// index is hit number
	std::map<std::string, std::vector<int>>         intVarsMap;
	std::map<std::string, std::vector<double>>      doubleVarsMap;
	std::map<std::string, std::vector<std::string>> stringVarsMap;

	/**
	 * \brief Register a variable branch with int vector storage.
	 * \param varname Variable / branch name.
	 * \param value Sample value used only to select overload (not stored).
	 */
	void registerVariable(const std::string& varname, int value);

	/**
	 * \brief Register a variable branch with double vector storage.
	 * \param varname Variable / branch name.
	 * \param value Sample value used only to select overload (not stored).
	 */
	void registerVariable(const std::string& varname, double value);

	/**
	 * \brief Register a variable branch with string vector storage.
	 * \param varname Variable / branch name.
	 * \param value Sample value used only to select overload (not stored).
	 */
	void registerVariable(const std::string& varname, const std::string& value);

	/// \brief Logger used for diagnostics; stored by reference to match existing ownership model.
	std::shared_ptr<GLogger>& log;
};
