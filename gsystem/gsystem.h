#pragma once

// gsystem
#include "gvolume.h"
#include "gmaterial.h"

// c++
#include <map>
#include <string>

/**
 * \ingroup gemc_gsystem_core
 *
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
 */
class GSystem : public GBase<GSystem>
{
public:
	/**
	 * \brief Construct a detector system descriptor.
	 *
	 * \param gopts Shared options/configuration instance.
	 * \param dbhost Database host (sqlite filename or remote host, depending on factory).
	 * \param sname Absolute or relative path including the system name (e.g. \c "detectors/ecal").
	 *             The name and path are parsed from this string.
	 * \param factory Factory label used to load this system (e.g. \c "ascii", \c "sqlite", \c "CAD", \c "GDML").
	 * \param experiment Experiment name used in DB filtering.
	 * \param runno Run number used in DB filtering.
	 * \param variation Variation string used in DB filtering and file naming.
	 * \param annotations Optional system annotations (e.g. \c "mats_only").
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
	 * \param other Source system.
	 *
	 * \details This constructor performs a deep copy of volumes and materials so that
	 * the copied system owns independent objects (while preserving value semantics).
	 */
	GSystem(const GSystem& other);

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
		return std::make_unique<GSystem>(*this);
	}

private:
	std::string dbhost;      ///< Database host (sqlite filename or remote DB host).
	std::string name;        ///< System name (parsed from \c sname).
	std::string path;        ///< Path portion parsed from \c sname (may be empty).
	std::string factoryName; ///< Factory label used to load this system.
	std::string experiment;  ///< Experiment name (DB filter).
	int         runno{};     ///< Run number (DB filter).
	std::string variation;   ///< Variation string (DB/file filter).
	std::string annotations; ///< Optional annotations (e.g. \c "mats_only").

	/**
	 * \name System content
	 * \brief Volumes and materials that define this detector subsystem.
	 *
	 * The keys of the maps are the logical identifiers (names) used in serialized geometry/material
	 * records. Factories are expected to enforce uniqueness within the loaded dataset.
	 */
	///@{
	std::map<std::string, std::shared_ptr<GVolume>>   gvolumesMap;   ///< Map of volume name → volume object.
	std::map<std::string, std::shared_ptr<GMaterial>> gmaterialsMap; ///< Map of material name → material object.
	///@}

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
	 *
	 * \details This is used primarily by file-based factories to resolve where to search
	 * for system resources (e.g. ASCII geometry/material files, CAD directories).
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
	 * \details This method is typically called only when the world volume is injected automatically.
	 * It creates a top-level volume whose mother marker is MOTHEROFUSALL.
	 */
	void addROOTVolume(const std::string& rootVolumeDefinition);

	/**
	 * \brief Build and add a volume from a serialized parameter list.
	 *
	 * \param pars Serialized volume parameter list.
	 *
	 * \details This method appends system-wide selectors (variation and run number)
	 * and then constructs a new volume, inserting it into the internal volume map.
	 *
	 * Error handling:
	 * - If the volume name key already exists, the method logs ERR_GVOLUMEALREADYPRESENT.
	 * - If the parameter vector size is wrong, the volume constructor logs ERR_GWRONGNUMBEROFPARS.
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
	 *
	 * The defaults are chosen so the imported asset is placeable:
	 * - mother is ROOTWORLDGVOLUMENAME
	 * - position and rotation are zero
	 * - material defaults to \c G4_AIR
	 * - visibility/style/color are set to visible + simple defaults
	 */
	void addVolumeFromFile(const std::string& importType, const std::string& filename);

	/**
	 * \brief Retrieve a volume by name.
	 *
	 * \param volumeName Volume name key.
	 * \return Pointer to the volume if found, otherwise \c nullptr.
	 *
	 * \details The returned pointer is non-owning and remains valid as long as the
	 * system retains the corresponding shared pointer in its volume map.
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
	 *
	 * \details
	 * - If the material name key is new, a material object is constructed and inserted.
	 * - If the key already exists, the method logs ERR_GMATERIALALREADYPRESENT.
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
	 *
	 * If the volume exists but the material does not, the method logs ERR_GMATERIALNOTFOUND.
	 */
	[[nodiscard]] const GMaterial* getMaterialForGVolume(const std::string& volumeName) const;
};

// using shared pointers as GSystem could be created from the main
// thread (in dbselect) then dispatched to g4 threads
using SystemPtr  = std::shared_ptr<GSystem>;
using SystemMap  = std::map<std::string, SystemPtr>;
using SystemList = std::vector<SystemPtr>;
