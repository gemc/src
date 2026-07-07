/**
 * \file loadGeometry.cc
 * \brief Geometry loading implementation for GSystemCADFactory.
 *
 * See systemCadFactory.h for API docs.
 */

// gsystem
#include "systemCadFactory.h"
#include "gsystemConventions.h"

// gemc
#include "gutilities.h"

// sqlite
#include "sqlite3.h"

// c++
#include <filesystem>
#include <unordered_map>

using namespace std;

namespace {
	/// \brief Check whether a column exists in the \c geometry table.
	bool geometry_column_exists(sqlite3* db, const std::string& column_name) {
		sqlite3_stmt* stmt = nullptr;
		if (sqlite3_prepare_v2(db, "SELECT name FROM PRAGMA_TABLE_INFO('geometry')", -1, &stmt, nullptr) != SQLITE_OK) {
			return false;
		}
		bool exists = false;
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			const unsigned char* colText = sqlite3_column_text(stmt, 0);
			if (colText != nullptr && column_name == reinterpret_cast<const char*>(colText)) {
				exists = true;
				break;
			}
		}
		sqlite3_finalize(stmt);
		return exists;
	}
}

void GSystemCADFactory::loadGeometry(GSystem* s) {
	// skip ROOT system
	if (s->getName() == ROOTWORLDGVOLUMENAME) { return; }

	// Resolve the directory holding the CAD meshes.
	string dirLocation = gutilities::searchForDirInLocations(s->getFilePath(), possibleLocationOfFiles);
	if (!filesystem::exists(dirLocation)) {
		log->error(ERR_GDIRNOTFOUND, "CAD Directory >" + s->getFilePath() + "< not found.");
		return;
	}

	// Map each available mesh file to its stem (filename without extension), which is the volume name.
	unordered_map<string, string> meshByName;
	for (const auto& cf : gutilities::getListOfFilesInDirectory(dirLocation, {".stl", ".ply"})) {
		meshByName[filesystem::path(cf).stem().string()] = cf;
	}

	// The list of volumes to load - and their metadata - comes from the sqlite database, not from the
	// directory listing: only meshes that have a matching row in the geometry table are imported.
	string dbhost = s->get_dbhost();
	if (dbhost.empty() || dbhost == "na") { dbhost = GSYSTEMSQLITETDEFAULTFILE; }

	vector<string> dirs = {
		".",
		gutilities::gemc_root().string(),
		(gutilities::gemc_root() / "examples").string()
	};
	auto dbPath = gutilities::searchForFileInLocations(dirs, dbhost);
	if (!dbPath) {
		log->error(ERR_GSQLITEERROR, "CAD factory: sqlite database <" + dbhost + "> not found.");
		return;
	}

	sqlite3* db = nullptr;
	if (sqlite3_open_v2(dbPath.value().c_str(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
		sqlite3_close(db);
		log->error(ERR_GSQLITEERROR, "CAD factory: failed to open sqlite database <" + dbhost + ">.");
		return;
	}
	log->info(1, "CAD factory: reading definitions from sqlite database ", dbPath.value());

	const string placement_column = geometry_column_exists(db, "g4placement_type")
	                                ? "g4placement_type"
	                                : "'" DEFAULTG4PLACEMENTTYPE "' AS g4placement_type";
	const string sql_query =
		"SELECT DISTINCT name, solid, parameters, material, mother, position, rotations, " +
		placement_column +
		", mfield, visible, style, color, opacity, digitization, identifier, copyOf, solidsOpr, mirror, "
		"exist, description FROM geometry WHERE experiment = ? AND system = ? AND variation = ? AND run = ?";

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db, sql_query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
		log->error(ERR_GSQLITEERROR, "CAD factory: error preparing query: ", sqlite3_errmsg(db));
		sqlite3_close(db);
		return;
	}

	string experiment  = s->getExperiment();
	string system_name = s->getName();
	string variation   = s->getVariation();
	int    runno       = s->getRunno();

	sqlite3_bind_text(stmt, 1, experiment.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, system_name.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 3, variation.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 4, runno);

	// The "description" column (last selected field) carries the mesh path for the g4 CAD builder.
	// The CAD factory resolves it against dirLocation so the database only needs the volume name.
	constexpr int DESCRIPTION_INDEX = 19;

	int loaded = 0;
	int rc;
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		vector<string> gvolumePars;
		const int      colCount = sqlite3_column_count(stmt);
		for (int i = 0; i < colCount; i++) {
			const unsigned char* colText = sqlite3_column_text(stmt, i);
			gvolumePars.emplace_back(colText ? reinterpret_cast<const char*>(colText) : "");
		}

		const string& volumeName = gvolumePars[0];
		const string& solidType  = gvolumePars[1];

		// Only CAD volumes are resolved against the mesh directory. A CAD volume defined in the
		// database but missing its mesh file is skipped with a warning.
		if (solidType == GSYSTEMCADTFACTORYLABEL) {
			auto it = meshByName.find(volumeName);
			if (it == meshByName.end()) {
				log->warning("CAD factory: volume <", volumeName,
				             "> is defined in the database but no mesh file was found in <",
				             dirLocation, ">; skipping.");
				continue;
			}
			gvolumePars[DESCRIPTION_INDEX] = dirLocation + "/" + it->second;
		}

		s->addGVolume(gvolumePars);
		loaded++;
	}

	if (rc != SQLITE_DONE) {
		log->error(ERR_GSQLITEERROR, "CAD factory: sqlite error while reading geometry: ", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);

	log->info(0, "CAD factory: loaded ", loaded, " volume(s) for system <", system_name,
	          ">, variation <", variation, ">, run ", runno,
	          " (", meshByName.size(), " mesh file(s) present in <", dirLocation, ">).");
}
