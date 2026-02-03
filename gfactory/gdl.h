#pragma once

// geomc
#include "glogger.h"
#include "gutilities.h"

// POSIX dynamic loading
// documentation: http://www.faqs.org/docs/Linux-mini/C++-dlopen.html
#include <dlfcn.h>

// c++
#include <sys/stat.h> // to check if file exists
#include <string>

/**
 * @typedef dlhandle
 * @brief Opaque handle returned by \c dlopen and consumed by \c dlsym / \c dlclose.
 */
typedef void* dlhandle;

static dlhandle load_lib(const std::string& path);
static void     close_lib(dlhandle handle);

// exit codes: 1000s
#define ERR_DLNOTFOUND       1001
#define ERR_FACTORYNOTFOUND  1002
#define ERR_DLHANDLENOTFOUND 1003


/**
 * @struct DynamicLib
 * @brief Helper that loads a shared library and holds its POSIX handle.
 *
 * This structure centralizes:
 * - *file discovery* (try user path, then GEMC install `lib/`, then `build/` for test runs),
 * - *opening* the library using \c dlopen, and
 * - *closing* the library using \c dlclose in the destructor.
 *
 * ### Search strategy
 * Given a library name or path:
 * 1. Try the provided value as-is.
 * 2. If not found, try `${GEMC_ROOT}/lib/<name>`.
 * 3. If still not found, try `${GEMC_ROOT}/build/<name>` (useful for Meson tests in the build tree).
 *
 * ### Error reporting
 * Errors are reported via the configured `GLogger` instance using:
 * - ERR_DLNOTFOUND when no file is found, and
 * - ERR_DLHANDLENOTFOUND when the file exists but \c dlopen returns null (with \c dlerror text).
 */
struct DynamicLib
{
private:
	/// Requested library file name (may be rewritten to include search paths).
	std::string dlFileName;

	/**
	 * @brief Check whether a file exists on disk.
	 * @param name Path to check.
	 * @return true if the file exists and `stat()` succeeds.
	 */
	bool doesFileExists(const std::string& name) {
		struct stat buffer{};
		return (stat(name.c_str(), &buffer) == 0);
	}

	/// Logger used for diagnostics and errors.
	std::shared_ptr<GLogger> log;

public:
	/// Default constructor (does not load anything).
	DynamicLib() = default;

	/**
	 * @brief Construct and attempt to load a dynamic library.
	 *
	 * @param logger Logger used for messages and errors.
	 * @param path   Library filename or path (e.g., `myplugin.gplugin`).
	 *
	 * The constructor attempts multiple locations (see class description) and, if found,
	 * opens the library via \c load_lib().
	 */
	DynamicLib(std::shared_ptr<GLogger> logger, std::string path) : dlFileName(path), log(logger), handle(nullptr) {
		log->debug(CONSTRUCTOR, "Instantiating ", path);
		log->debug(NORMAL, "Trying ", dlFileName);

		// Try installation path + "/lib" if not found at the provided location.
		if (!doesFileExists(dlFileName)) {
			log->debug(NORMAL, dlFileName, " not found...");

			std::filesystem::path gemcRoot = gutilities::gemc_root();
			dlFileName                     = gemcRoot.string() + "/lib/" + path;

			log->debug(NORMAL, "Trying ", dlFileName);
		}

		// Try installation path + "/build" if not found - allows Meson tests to run from the build dir.
		if (!doesFileExists(dlFileName)) {
			log->debug(NORMAL, dlFileName, " not found...");

			std::filesystem::path gemcRoot = gutilities::gemc_root();
			dlFileName                     = gemcRoot.string() + "/build/" + path;

			log->debug(NORMAL, "Trying ", dlFileName);
		}

		if (doesFileExists(dlFileName)) {
			handle = load_lib(dlFileName);
			if (handle == nullptr) {
				char const* const dlopen_error = dlerror();
				log->error(ERR_DLHANDLENOTFOUND, "File ", dlFileName, " found, but handle is null. dlopen_error >> ",
				           dlopen_error);
			}
			else { log->info(0, "Loaded ", dlFileName); }
		}
		else { log->error(ERR_DLNOTFOUND, "could not find ", dlFileName); }
	}

	/**
	 * @brief POSIX handle of the dynamic library.
	 *
	 * This is the value returned by \c dlopen and is later consumed by \c dlsym.
	 */
	dlhandle handle = nullptr;

	/**
	 * @brief Destructor closes the library handle (if loaded).
	 *
	 * @note \c dlclose is called only if \ref handle "handle" is not null.
	 */
	~DynamicLib() {
		if (handle != nullptr) {
			close_lib(handle);
			log->debug(DESTRUCTOR, "Destroying ", dlFileName);
		}
	}
};


/**
 * @brief Open a shared library using \c dlopen.
 *
 * This helper attempts:
 * - direct open if the caller supplied an explicit path (contains `/`), else
 * - open from the current working directory (`./<lib>`), then
 * - fall back to the system search path (`LD_LIBRARY_PATH`, RPATH/RUNPATH, system dirs).
 *
 * @param lib Library name or path.
 * @return A non-null handle on success; null on failure. Caller may inspect \c dlerror.
 */
dlhandle load_lib(const std::string& lib) // never throws
{
	dlhandle h = nullptr;

	// If the caller already supplied a path (has a slash) just try it.
	if (lib.find('/') != std::string::npos) { h = dlopen(lib.c_str(), RTLD_NOW); }
	else {
		// 1. Try the file in the current working directory.
		std::string cwdPath = "./" + lib;
		h                   = dlopen(cwdPath.c_str(), RTLD_NOW);

		// 2. Fallback to the normal search path so LD_LIBRARY_PATH,
		//    RPATH/RUNPATH, system dirs, etc. are still honoured.
		if (!h) { h = dlopen(lib.c_str(), RTLD_NOW); }
	}
	return h; // may be nullptr – caller should check and use dlerror()
}

/**
 * @brief Close a library handle using \c dlclose.
 * @param handle Handle returned by \c dlopen.
 */
static void close_lib(dlhandle handle) { dlclose(handle); }
