#pragma once

// gsystem
#include "gvolume.h"
#include "gmaterial.h"

// c++
#include <map>
#include <string>

/**
 * \class GSystem
 * \brief Represents a single detector system (e.g., calorimeter, tracker).
 *
 * A GSystem is a container for:
 * - a set of geometry volumes (GVolume) that define shapes/placements;
 * - a set of materials (GMaterial) that volumes refer to by name.
 *
 * Systems are typically created from configuration (GOptions) and then populated by a
 * factory (derived from GSystemFactory) that loads geometry/material rows from a source
 * (sqlite DB, ASCII, CAD import, GDML import, etc.).
 *
 * Key configuration dimensions:
 * - \c experiment : filters DB queries and system selection
 * - \c variation  : selects a geometry/material variant
 * - \c runno      : selects a run-dependent configuration
 *
 * \note For classes not included in this module, do not use \ref (tag files handle linking).
 */
class GSystem : public GBase<GSystem>
{
public:
	/**
	 * \brief Construct a detector system descriptor.
	 *
	 * \param gopts Shared options/configuration instance.
	 * \param dbhost Database host (sqlite filename or remote host, depending on factory).
	 * \param sname Absolute or relative path including the system name (e.g. "detectors/ecal").
	 *             The name and path are parsed from this string.
	 * \param factory Factory label used to load this system (e.g. "ascii", "sqlite", "CAD", "GDML").
	 * \param experiment Experiment name used in DB filtering.
	 * \param runno Run number used in DB filtering.
	 * \param variation Variation string used in DB filtering and file naming.
	 * \param annotations Optional system annotations (e.g. "mats_only").
	 */
	GSystem(const std::shared_ptr<GOptions>& gopts,
	        const std::string&               dbhost,
	        const std::string&               sname,
	        const std::string&               factory,
	        const std::string&               experiment,
	        int                              runno,
	        const std::string&               variation,
	        const std::string&               annotations = "none"
	);


	/**
	 * \brief Deep copy constructor (used only by clone()).
	 *
	 * \note The intent is to duplicate volumes/materials so that systems can be copied safely.
	 * See implementation for the cloning strategy.
	 */
	GSystem(const GSystem& other); // copy constructor

	// move operations: defaulted
	GSystem(GSystem&&) noexcept            = default;
	GSystem& operator=(GSystem&&) noexcept = default;

	// **disable copying**
	GSystem& operator=(const GSystem&) = delete;

	/**
	 * \brief Clone helper.
	 *
	 * \return A deep-copied system instance.
	 */
	[[nodiscard]] std::unique_ptr<GSystem> clone() const {
		return std::make_unique<GSystem>(*this); // invokes copy-ctor
	}

private:
	std::string dbhost;      ///< Database host (sqlite filename or remote DB host).
	std::string name;        ///< System name (parsed from sname).
	std::string path;        ///< Path portion parsed from sname (may be empty).
	std::string factoryName; ///< Factory label used to load this system.
	std::string experiment;  ///< Experiment name (DB filter).
	int         runno{};     ///< Run number (DB filter).
	std::string variation;   ///< Variation string (DB/file filter).
	std::string annotations; ///< Optional annotations (e.g. "mats_only").

	/// Map of volume name → volume object.
	/// The key must be unique across the full world; factories are expected to enforce uniqueness.
	std::map<std::string, std::shared_ptr<GVolume>> gvolumesMap;

	/// Map of material name → material object for this system.
	std::map<std::string, std::shared_ptr<GMaterial>> gmaterialsMap;

public:
	/// \name System metadata
	///@{
	[[nodiscard]] inline std::string getName() const { return name; }
	[[nodiscard]] inline std::string getFactoryName() const { return factoryName; }
	[[nodiscard]] inline std::string getVariation() const { return variation; }
	[[nodiscard]] inline std::string getExperiment() const { return experiment; }
	[[nodiscard]] inline std::string getAnnotations() const { return annotations; }
	[[nodiscard]] inline int         getRunno() const { return runno; }
	[[nodiscard]] inline std::string get_dbhost() const { return dbhost; }
	///@}

	/**
	 * \brief Gets the full file path of the system.
	 *
	 * \return If \c path is empty, returns \c name; otherwise returns \c path + "/" + name.
	 */
	std::string getFilePath();

	/**
	 * \brief Sets the database host.
	 *
	 * This is used by front-ends (e.g. GUI) to override the DB location after construction.
	 *
	 * \param dbh New database host/filename.
	 */
	inline void set_dbhost(const std::string& dbh) { this->dbhost = dbh; }

	/**
	 * \brief Adds the special ROOT/world volume to the system.
	 *
	 * \param rootVolumeDefinition The ROOT/world definition string (solid + parameters + material).
	 *
	 * \note This is typically called only when the world volume is injected automatically.
	 */
	void addROOTVolume(const std::string& rootVolumeDefinition);

	/**
	 * \brief Build and add a volume from a serialized parameter list.
	 *
	 * \param pars Serialized volume parameter list.
	 *
	 * \details This method appends system-wide selectors (variation and run number)
	 * and then constructs a new volume, inserting it into the internal volume map.
	 */
	void addGVolume(std::vector<std::string> pars);

	/**
	 * \brief Add a volume imported from a file (CAD, GDML, etc.).
	 *
	 * \param importType Import label (factory-specific).
	 * \param filename Full filename (including path) of the imported geometry asset.
	 *
	 * \details This builds a synthetic parameter vector with reasonable defaults
	 * and forwards it to \ref GSystem::addGVolume "addGVolume()".
	 */
	void addVolumeFromFile(const std::string& importType, const std::string& filename);

	/**
	 * \brief Retrieve a volume by name.
	 *
	 * \param volumeName Volume name key.
	 * \return Pointer to the volume if found, otherwise \c nullptr.
	 */
	[[nodiscard]] GVolume* getGVolume(const std::string& volumeName) const;

	/**
	 * \brief Access all volumes in this system.
	 *
	 * \return Const reference to the internal volume map.
	 */
	[[nodiscard]] inline const std::map<std::string, std::shared_ptr<GVolume>>& getGVolumesMap() const {
		return gvolumesMap;
	}

	/**
	 * \brief Access all materials in this system.
	 *
	 * \return Const reference to the internal material map.
	 */
	[[nodiscard]] inline const std::map<std::string, std::shared_ptr<GMaterial>>& getGMaterialMap() const {
		return gmaterialsMap;
	}

	/**
	 * \brief Build and add a material from a serialized parameter list.
	 *
	 * \param pars Serialized material parameter list.
	 */
	void addGMaterial(std::vector<std::string> pars);

	/**
	 * \brief Retrieve the material associated with a given volume.
	 *
	 * \param volumeName Name of the volume whose material is requested.
	 * \return Pointer to the material if found, otherwise \c nullptr.
	 *
	 * \details The lookup is performed by:
	 * - locating the volume in the volume map;
	 * - extracting its material name;
	 * - locating the material in the material map.
	 */
	[[nodiscard]] const GMaterial* getMaterialForGVolume(const std::string& volumeName) const;
};

// using shared pointers as GSystem could be created from the main
// thread (in dbselect) then dispatched to g4 threads
using SystemPtr  = std::shared_ptr<GSystem>;
using SystemMap  = std::map<std::string, SystemPtr>;
using SystemList = std::vector<SystemPtr>;
