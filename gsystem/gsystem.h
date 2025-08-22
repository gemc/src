#pragma once

// gsystem
#include "gvolume.h"
#include "gmaterial.h"

// c++
#include <map>
#include <string>


/**
 * @class GSystem
 * @brief Represents a single detector system (e.g., calorimeter, tracker).
 *
 * A GSystem acts as a container for a set of GVolume objects and their
 * corresponding GMaterial definitions. It groups components belonging to the
 * same logical detector part and manages their retrieval. Each system is
 * identified by a name, factory, variation, and run number.
 */
class GSystem : public GBase<GSystem> {

public:
	/**
	 * @brief Constructs a GSystem instance.
	 * @param gopts A shared pointer to GOptionsy.
	 * @param dbhost The database host (e.g., "gemc.db" or "localhost").
	 * @param sname Absolute or relative path including the system name (e.g., "detectors/ecal").
	 *                       The name and path will be parsed from this string.
	 * @param factory The name of the factory responsible for building this system (e.g., "TEXT", "GDML").
	 * @param experiment The name of the experiment (e.g., "clas12").
	 * @param variation The variation identifier for this system configuration (e.g., "default", "v2").
	 * @param runno The run number this configuration applies to.
	 * @param annotations Optional descriptive annotations (e.g., "mats_only").
	 */
	GSystem(const std::shared_ptr<GOptions>& gopts,
	        const std::string&              dbhost,
	        const std::string&              sname,
	        const std::string&              factory,
	        const std::string&              experiment,
	        int                             runno,
	        const std::string&              variation,
	        const std::string&              annotations = "none"
	);


	/**
	 * @brief Deep copy constructor (used only by clone())
	 */
	GSystem(const GSystem& other); // copy constructor

	// move operations: defaulted
	GSystem(GSystem&&) noexcept            = default;
	GSystem& operator=(GSystem&&) noexcept = default;

	// **disable copying**
	GSystem& operator=(const GSystem&) = delete;

	// simple deep‑copy helper
	[[nodiscard]] std::unique_ptr<GSystem> clone() const {
		return std::make_unique<GSystem>(*this); // invokes copy‑ctor
	}


private:
	std::string              dbhost;      ///< Database host (if using sqlite or mysql).
	std::string              name;        ///< System name.
	std::string              path;        ///< Absolute/relative path.
	std::string              factoryName; ///< Name of the factory that builds the detector.
	std::string              experiment;  ///< Experiment name (e.g., "clas12").
	int                      runno{};     ///< Run number.
	std::string              variation;   ///< Variation of the detector.
	std::string              annotations; ///< Annotations (e.g., "mats_only" means only materials are loaded).

	/// Map containing the volumes.
	/// The key is a unique volume name (system and volume name).
	std::map<std::string, std::shared_ptr<GVolume>> gvolumesMap;

	/// Map containing the materials for the system.
	std::map<std::string, std::shared_ptr<GMaterial>> gmaterialsMap;

public:
	/// \brief Gets the system name.
	[[nodiscard]] inline std::string getName() const { return name; }

	/// \brief Gets the factory name.
	[[nodiscard]] inline std::string getFactoryName() const { return factoryName; }

	/// \brief Gets the detector variation.
	[[nodiscard]] inline std::string getVariation() const { return variation; }

	/// \brief Gets the experiment name.
	[[nodiscard]] inline std::string getExperiment() const { return experiment; }

	/**
	 * \brief Gets the full file path of the system.
	 * \return The concatenated path and file name.
	 */
	std::string getFilePath();

	/// \brief Gets the annotations.
	[[nodiscard]] inline std::string getAnnotations() const { return annotations; }

	/// \brief Gets the run number.
	[[nodiscard]] inline int getRunno() const { return runno; }

	/// \brief Gets the database host.
	[[nodiscard]] inline std::string get_dbhost() const { return dbhost; }

	/// \brief Sets the database host. For example, the GUI can reset this
	inline void set_dbhost(const std::string& dbh) { this->dbhost = dbh; }

	/**
	 * \brief Adds a ROOT volume to the system.
	 * \param rootVolumeDefinition The definition string for the ROOT volume.
	 */
	void addROOTVolume(const std::string& rootVolumeDefinition);

	/**
	 * \brief Adds a GVolume to the system using a set of parameters.
	 * \param pars A vector of strings representing volume parameters.
	 */
	void addGVolume(std::vector<std::string> pars);

	/**
	 * \brief Adds a volume from a file (e.g., CAD or GDML).
	 * \param importType The type of import.
	 * \param filename The file containing the volume definition.
	 */
	void addVolumeFromFile(const std::string& importType, const std::string& filename);

	/**
	 * \brief Retrieves a GVolume by its volume name.
	 * \param volumeName The name of the volume.
	 * \return Pointer to the GVolume if found, or nullptr otherwise.
	 */
	[[nodiscard]] GVolume* getGVolume(const std::string& volumeName) const;

	/// \brief Returns the map of volumes.
	[[nodiscard]] inline const std::map<std::string, std::shared_ptr<GVolume>>& getGVolumesMap() const { return gvolumesMap; }

	/// \brief Returns the map of materials.
	[[nodiscard]] inline const std::map<std::string, std::shared_ptr<GMaterial>>& getGMaterialMap() const { return gmaterialsMap; }


	/**
	 * \brief Adds a GMaterial to the system using a set of parameters.
	 * \param pars A vector of strings representing material parameters.
	 */
	void addGMaterial(std::vector<std::string> pars);

	/**
	 * \brief Retrieves the material associated with a given volume.
	 * \param volumeName The name of the volume.
	 * \return Pointer to the GMaterial if found, or nullptr otherwise.
	 */
	[[nodiscard]] const GMaterial* getMaterialForGVolume(const std::string& volumeName) const;

};

// using shared pointers as GSystem could be created from the main
// thread (in dbselect) then dispatched to g4 threads
using SystemPtr  = std::shared_ptr<GSystem>;
using SystemMap  = std::map<std::string, SystemPtr>;
using SystemList = std::vector<SystemPtr>;
