#pragma once

// gemc
#include "gfield.h"
#include "gfield_options.h"
#include "gfieldConventions.h"

/**
 * @defgroup gfield_module GField module
 *
 * @brief Plugin-driven magnetic-field framework.
 *
 * The module provides:
 * - A common abstract interface for magnetic fields (\ref GField),
 * - A lightweight configuration carrier (\ref GFieldDefinition),
 * - A manager that loads plugins and builds \c G4FieldManager instances (\ref GMagneto),
 * - Option definitions for fields implemented in this module (namespace gfields).
 *
 * The actual magnetic-field behavior is implemented in plugins (shared libraries) that expose a
 * C factory symbol (see \ref GField::instantiate "instantiate()").
 */

/**
 * @brief Manager for magnetic fields and their associated \c G4FieldManager objects.
 * @ingroup gfield_module
 *
 * Ownership model:
 * - \ref GMagneto "GMagneto" owns a map of field objects (\ref GField) created via dynamic plugin loading.
 * - For each field it also owns a corresponding \c G4FieldManager created by \ref GField::create_FieldManager "create_FieldManager()".
 *
 * Lifecycle:
 * - Fields and managers are constructed during \ref GMagneto::GMagneto "GMagneto()" based on the field definitions
 *   produced by gfields::get_GFieldDefinition().
 * - Maps live for the lifetime of the \ref GMagneto "GMagneto" instance.
 */
class GMagneto : public GBase<GMagneto> {
public:
	/**
	 * @brief Construct and initialize the magnetic field registry.
	 * @param gopts Shared options used for configuration and logging.
	 *
	 * This constructor:
	 * 1. Builds the list of \ref GFieldDefinition "GFieldDefinition" objects from options,
	 * 2. Loads each corresponding plugin library using the factory manager,
	 * 3. Instantiates the field and calls \ref GField::load_field_definitions "load_field_definitions()",
	 * 4. Creates and stores a \c G4FieldManager via \ref GField::create_FieldManager "create_FieldManager()".
	 */
	explicit GMagneto(const std::shared_ptr<GOptions>& gopts);

private:
	using gFieldMap    = std::unordered_map<std::string, std::shared_ptr<GField>>;
	using gFieldMgrMap = std::unordered_map<std::string, std::shared_ptr<G4FieldManager>>;

	/// Map of user field names to instantiated field objects.
	std::shared_ptr<gFieldMap> fields_map;

	/// Map of user field names to their corresponding \c G4FieldManager objects.
	std::shared_ptr<gFieldMgrMap> fields_manager;

public:
	/**
	 * @brief Check whether a field with the given name exists.
	 * @param name Field name key.
	 * @return True if \c name is present in the internal field map.
	 */
	bool isField(const std::string& name) const { return fields_map->find(name) != fields_map->end(); }

	/**
	 * @brief Retrieve a field object by name.
	 * @param name Field name key.
	 * @return Shared pointer to the requested \ref GField "GField" instance.
	 *
	 * If the field is not found, an error is logged and execution is terminated by the logger.
	 */
	std::shared_ptr<GField> getField(std::string name) {
		bool not_found = (fields_map->find(name) == fields_map->end());

		if (not_found) { log->error(ERR_WRONG_FIELD_NOT_FOUND, "GField >", name, "< not found. Exiting."); }
		return fields_map->at(name);
	}

	/**
	 * @brief Retrieve the \c G4FieldManager associated with a given field name.
	 * @param name Field name key.
	 * @return Shared pointer to the requested \c G4FieldManager.
	 *
	 * If the field manager is not found, an error is logged and execution is terminated by the logger.
	 */
	std::shared_ptr<G4FieldManager> getFieldMgr(std::string name) {

		// Debug visibility: print out the full map of registered field managers.
		for (const auto& [key, value] : *fields_manager) {
			log->info(2, "GFieldManager >", key, "< >", value);
		}

		if (fields_manager->find(name) == fields_manager->end()) { log->error(ERR_WRONG_FIELD_NOT_FOUND, "GField >", name, "< not found. Exiting."); }

		return fields_manager->at(name);
	}
};
