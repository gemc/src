#pragma once

// gsystem
#include "systemFactory.h"

// sqlite
#include "sqlite3.h"

/**
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
 * Special-case:
 * - The ROOT/world system is skipped by design (it is injected separately).
 *
 * \note Uses \c sqlite3 types directly; external library symbols are not referenced with \ref.
 */
class GSystemSQLiteFactory : public GSystemFactory
{
public:
	// inherit the base (const std::shared_ptr<GOptions>&) ctor
	using GSystemFactory::GSystemFactory;

private:
	/// \brief Load materials for the requested system from sqlite.
	void loadMaterials(GSystem* system) override;

	/// \brief Load geometry volumes for the requested system from sqlite.
	void loadGeometry(GSystem* system) override;

	/// \brief Close and reset the sqlite handle and clear search state.
	void closeSystem() override;

	/**
	 * \brief Helper: check whether a table exists in the sqlite DB.
	 *
	 * \param db Open sqlite connection.
	 * \param name Table name.
	 * \return \c true if the table exists, \c false otherwise.
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
	 */
	void initialize_sqlite_db(GSystem* system);

	sqlite3*    db          = nullptr;   ///< Open sqlite DB connection (read-only).
	std::string system_name = "na";      ///< Cached system name used in logs and DB filtering.
	std::string dbhost      = "na";      ///< DB filename (or host string; sqlite uses filename).
	std::string variation   = "default"; ///< Cached variation.
	int         runno       = 0;         ///< Cached run number.
};
