#ifndef  GSYSTEM_H
#define  GSYSTEM_H 1

// gsystem
#include "gvolume.h"
#include "gmaterial.h"

// gemc
#include "glogger.h"

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
class GSystem {

public:

	/**
	 * @brief Constructs a GSystem instance.
	 * @param systemFullName Absolute or relative path including the system name (e.g., "detectors/ecal").
	 *                       The name and path will be parsed from this string.
	 * @param factory The name of the factory responsible for building this system (e.g., "TEXT", "GDML").
	 * @param variation The variation identifier for this system configuration (e.g., "default", "v2").
	 * @param run The run number this configuration applies to.
	 * @param annotations Optional descriptive annotations (e.g., "mats_only").
	 * @param logger A shared pointer to the logging facility.
	 */
	GSystem(std::shared_ptr<GLogger> logger,
			const std::string &givenname,
			std::string factory,
			std::string variation,
			int runno = 0,
			std::string annotations = "none"
	);

	/**
	 * @brief Constructs a GSystem instance from another GSystem.
	 * @param gsystem The GSystem to copy.
	 */
	GSystem(const GSystem &gsystem) = default;


	~GSystem() {
		log->debug(DESTRUCTOR, "GSystem");
	}

private:

	std::string name;                    ///< System name.
	std::string path;                    ///< Absolute/relative path.
	std::string factoryName;             ///< Name of factory that builds the detector.
	std::string variation;               ///< Variation of the detector. Default is "default".
	int runno;                           ///< Run number.
	std::string annotations;             ///< Annotations (e.g., "mats_only" means only materials are loaded).

	/// Map containing the volumes.
	/// The key is a unique volume name (system + volume name).
	std::map<std::string, std::unique_ptr<GVolume>> gvolumesMap;


	/// Map containing the materials for the system.
	std::map<std::string, std::unique_ptr<GMaterial>> gmaterialsMap;

	std::string dbhost;                  ///< Database host (if using sqlite or mysql).
	std::shared_ptr<GLogger> log;        ///< Logger instance for logging messages.

public:
	/// \brief Gets the system name.
	[[nodiscard]] inline std::string getName() const { return name; }

	/// \brief Gets the factory name.
	[[nodiscard]]    inline std::string getFactoryName() const { return factoryName; }

	/// \brief Gets the detector variation.
	[[nodiscard]] inline std::string getVariation() const { return variation; }

	/**
	 * \brief Gets the full file path of the system.
	 * \return The concatenated path and file name.
	 */
	std::string getFilePath();

	/// \brief Gets the annotations.
	[[nodiscard]] inline std::string getAnnotations() const { return annotations; }

	/// \brief Gets the run number.
	[[nodiscard]]    inline int getRunno() const { return runno; }

	/// \brief Gets the database host.
	[[nodiscard]] inline  std::string get_dbhost() const { return dbhost; }

	/// \brief Sets the database host.
	inline void set_dbhost(const std::string &dbh) { this->dbhost = dbh; }

	/**
	 * \brief Adds a ROOT volume to the system.
	 * \param rootVolumeDefinition The definition string for the ROOT volume.
	 */
	void addROOTVolume(const std::string &rootVolumeDefinition);

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
	void addVolumeFromFile(const std::string &importType, const std::string &filename);

	/**
	 * \brief Retrieves a GVolume by its volume name.
	 * \param volumeName The name of the volume.
	 * \return Pointer to the GVolume if found, or nullptr otherwise.
	 */
	[[nodiscard]] GVolume *getGVolume(const std::string &volumeName) const;

	/// \brief Returns the map of volumes.
	[[nodiscard]] inline const std::map<std::string, std::unique_ptr<GVolume>> &getGVolumesMap() const { return gvolumesMap; }

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
	[[nodiscard]] const GMaterial *getMaterialForGVolume(const std::string &volumeName) const;


};


#endif
