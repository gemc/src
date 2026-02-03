#pragma once

// gemc
#include "gelectronic.h"
#include "gbase.h"

// c++
#include <string>
#include <unordered_map>
#include <vector>

// tt
#include "gtranslationTable_options.h"

/**
 * \class GTranslationTable
 * \brief Stores and retrieves GElectronic configurations by a vector-based identity.
 *
 * A Translation Table maps an identity expressed as a \c std::vector<int> to a GElectronic object.
 * Internally, the identity vector is converted into a **stable string key** (hyphen-separated integers),
 * which is then used as the key in an \c std::unordered_map.
 *
 * Design goals:
 * - Provide a simple, fast lookup for electronics configurations.
 * - Keep identity handling explicit and deterministic (the same vector always yields the same key).
 * - Emit useful logs for both normal operation and debugging.
 *
 * Error handling:
 * - If a key already exists when inserting, the module logs a warning and preserves the original value.
 * - If a key is not found when retrieving, the module logs an error (EC__TTNOTFOUNDINTT) and returns
 *   a default-constructed GElectronic.
 *
 * \note This class derives from GBase to obtain consistent module logging behavior and to bind the
 *       logger name (TRANSLATIONTABLE_LOGGER).
 */
class GTranslationTable : public GBase<GTranslationTable> {

public:
	/**
	 * \brief Constructs a translation table bound to the provided options/logger configuration.
	 *
	 * The options are used to configure logging and any module-level behavior supported by the project
	 * options infrastructure.
	 *
	 * \param gopt Shared pointer to the project options object used by this module.
	 */
	explicit GTranslationTable(const std::shared_ptr<GOptions>& gopt)
		: GBase(gopt, TRANSLATIONTABLE_LOGGER) {  }

	/**
	 * \brief Registers an electronics configuration for a given identity vector.
	 *
	 * This method converts \p identity into the internal key representation and attempts to insert
	 * \p gtron into the table.
	 *
	 * Insertion rules:
	 * - If the derived key does not exist, the entry is inserted.
	 * - If the derived key already exists, the entry is **not** overwritten; a warning is logged.
	 *
	 * Logging behavior:
	 * - Emits informational output at verbosity level 1 after attempting the insertion.
	 * - May emit debug output with the full table content when debug logging is enabled.
	 *
	 * \param identity A vector of integers representing the unique identity.
	 * \param gtron The GElectronic configuration to associate with \p identity.
	 */
	void addGElectronicWithIdentity(const std::vector<int>& identity, const GElectronic& gtron);

	/**
	 * \brief Retrieves the electronics configuration associated with a given identity vector.
	 *
	 * This method derives the internal key from \p identity and looks it up in the table.
	 *
	 * - If found, the associated GElectronic is returned.
	 * - If not found, an error is logged (EC__TTNOTFOUNDINTT) and a default-constructed GElectronic is returned.
	 *
	 * \param identity A vector of integers representing the unique identity.
	 * \return The GElectronic associated with \p identity, or a default-constructed GElectronic if not found.
	 */
	[[nodiscard]] GElectronic getElectronics(const std::vector<int>& identity) const;

private:
	// The map uses a string formed from the identity vector as its key.
	// Key format: hyphen-separated integers (e.g. "1-2-3-4-5").
	std::unordered_map<std::string, GElectronic> tt;

	/**
	 * \brief Forms the internal translation table key from an identity vector.
	 *
	 * The key is formed by concatenating each integer value separated by a hyphen:
	 * \code
	 * identity = {1, 2, 3}  ->  "1-2-3"
	 * \endcode
	 *
	 * Edge cases:
	 * - If \p identity is empty, the implementation logs a warning and returns an empty string.
	 *
	 * \param identity A vector of integers representing the identity.
	 * \return A string key representing \p identity in a stable, hyphen-separated form.
	 */
	[[nodiscard]] std::string formTTKey(const std::vector<int>& identity) const;

};
