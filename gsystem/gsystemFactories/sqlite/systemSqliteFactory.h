#pragma once

// gsystem
#include "systemFactory.h"

// sqlite
#include "sqlite3.h"

/**
 * \ingroup gemc_gsystem_factories_sqlite
 *
 * \class GSystemSQLiteFactory
 * \brief Load a GSystem from a sqlite database.
 *
 * This factory populates:
 * - materials from the \c materials table;
 * - geometry volumes from the \c geometry table.
 *
 * Database lifetime:
 * - The sqlite connection is created on-demand during the first load call.
 * - The DB handle is closed in \ref GSystemSQLiteFactory::closeSystem "closeSystem()".
 *
 * Special-case behavior:
 * - The ROOT/world system (ROOTWORLDGVOLUMENAME) is skipped by design (it is injected separately).
 *
 */
class GSystemSQLiteFactory : public GSystemFactory
{
public:
	// inherit the base (const std::shared_ptr<GOptions>&) ctor
	using GSystemFactory::GSystemFactory;

private:
	/**
	 * \brief Load materials for the requested system from sqlite.
	 *
	 * \param system Target system to populate.
	 *
	 * \details
	 * - If the \c materials table is empty, this is treated as valid (materials may come from \c G4Material).
	 * - Rows are read as positional fields and forwarded to the system material builder.
	 */
	void loadMaterials(GSystem* system) override;

	/**
	 * \brief Load geometry volumes for the requested system from sqlite.
	 *
	 * \param system Target system to populate.
	 *
	 * \details Geometry rows are selected by experiment/system/variation/run and the positional
	 * fields beyond the metadata columns are forwarded to the system volume builder.
	 */
	void loadGeometry(GSystem* system) override;

	/**
	 * \brief Close and reset the sqlite handle and clear search state.
	 *
	 * \details This closes the read-only sqlite handle (if open), resets internal cached state,
	 * and clears \c possibleLocationOfFiles.
	 */
	void closeSystem() override;

	/**
	 * \brief Helper: check whether a table exists in the sqlite DB.
	 *
	 * \param db Open sqlite connection.
	 * \param name Table name.
	 * \return \c true if the table exists, \c false otherwise.
	 *
	 * \details This is a lightweight query against \c sqlite_master.
	 */
	bool table_exists(sqlite3* db, const char* name);

	/**
	 * \brief Open the sqlite database and cache run/variation metadata.
	 *
	 * \param system Target system whose (name, variation, runno) are cached.
	 *
	 * \details The method searches for the DB file in:
	 * - current directory
	 * - GEMC installation root
	 * - GEMC examples directory
	 *
	 * The first match is opened read-only and stored in \c db.
	 */
	void initialize_sqlite_db(GSystem* system);

	sqlite3*    db          = nullptr;   ///< Open sqlite DB connection (read-only).
	std::string system_name = "na";      ///< Cached system name used in logs and DB filtering.
	std::string dbhost      = "na";      ///< DB filename (sqlite uses filename).
	std::string variation   = "default"; ///< Cached variation.
	int         runno       = 0;         ///< Cached run number.
};
