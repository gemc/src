#pragma once

// gemc
#include "gfield.h"
#include "gfieldConventions.h"

/**
 * @brief Represents a world containing collections of GFields and G4FieldManagers.
 *
 * GMagneto manages the lifecycle and access to magnetic field objects (`GField`) and their
 * corresponding field managers (`G4FieldManager`).
 */
class GMagneto {
public:
	/**
	 * @brief Constructs a GMagneto object and loads field definitions from options.
	 * @param gopts Pointer to the GOptions object containing configuration options.
	 */
	explicit GMagneto(std::shared_ptr<GOptions> gopts);

	/**
	* @brief Destructor to clean up dynamically allocated memory.
	*/
	~GMagneto() { log->debug(DESTRUCTOR, "GMagneto"); }

private:
	using gFieldMap    = std::unordered_map<std::string, std::shared_ptr<GField>>;
	using gFieldMgrMap = std::unordered_map<std::string, std::shared_ptr<G4FieldManager>>;

	std::shared_ptr<gFieldMap>    fields_map;     ///< Map of field names to GField objects.
	std::shared_ptr<gFieldMgrMap> fields_manager; ///< Map of field names to G4FieldManager objects.

	/// Pointer to a logger instance for logging messages.
	std::shared_ptr<GLogger> log;

public:
	/**
	 * @brief Checks if a field with the given name exists.
	 * @param name Name of the field to check.
	 * @return True if the field exists, false otherwise.
	 */
	bool isField(const std::string& name) const { return fields_map->find(name) != fields_map->end(); }


	/**
	 * @brief Retrieves a GField object by its name.
	 * @param name Name of the field to retrieve.
	 * @return Pointer to the GField object.
	 * @throws Logs an error and exits if the field is not found.
	 */
	std::shared_ptr<GField> getField(std::string name) {
		bool not_found = (fields_map->find(name) == fields_map->end());

		if (not_found) { log->error(ERR_WRONG_FIELD_NOT_FOUND, "GField ", name, " not found. Exiting."); }
		return fields_map->at(name);
	}

	/**
	 * @brief Retrieves a G4FieldManager object by its name.
	 * @param name Name of the field manager to retrieve.
	 * @return Pointer to the G4FieldManager object.
	 * @throws Logs an error and exits if the field manager is not found.
	 */
	std::shared_ptr<G4FieldManager> getFieldMgr(std::string name) {
		if (fields_manager->find(name) == fields_manager->end()) { log->error(ERR_WRONG_FIELD_NOT_FOUND, "GField ", name, " not found. Exiting."); }

		return fields_manager->at(name);
	}

};
