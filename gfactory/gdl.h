#pragma once

// geomc
#include "glogger.h"
#include "gutilities.h"

// c++ plugin loading functions
// documentation: http://www.faqs.org/docs/Linux-mini/C++-dlopen.html
#include <dlfcn.h>

// c++
#include <sys/stat.h> // to check if file exists
#include <string>

typedef void* dlhandle;

static dlhandle load_lib(const std::string& path);

static void close_lib(dlhandle handle);

// exit codes: 1000s
#define ERR_DLNOTFOUND       1001
#define ERR_FACTORYNOTFOUND  1002
#define ERR_DLHANDLENOTFOUND 1003


/**
 * @struct DynamicLib
 * @brief  Structure to load dynamically symbols from a shared library
 */
struct DynamicLib {

private:
	std::string dlFileName; // dynamic library file

	bool doesFileExists(const std::string& name) {
		struct stat buffer{};
		return (stat(name.c_str(), &buffer) == 0);
	}

	std::shared_ptr<GLogger> log;

public:
	// default constructor
	DynamicLib() = default;

	// path here is the filename
	DynamicLib(std::shared_ptr<GLogger> logger, std::string path) : dlFileName(path), log(logger), handle(nullptr) {
		log->debug(CONSTRUCTOR, "Instantiating ", path);
		log->debug(NORMAL, "Trying ", dlFileName);

		// trying installation path + lib if not found
		if (!doesFileExists(dlFileName)) {
			log->debug(NORMAL, dlFileName, " not found...");

			std::filesystem::path gemcRoot = gutilities::gemc_root();
			dlFileName = gemcRoot.string() + "/lib/" + path;

			log->debug(NORMAL, "Trying ", dlFileName);
		}

		if (doesFileExists(dlFileName)) {
			handle = load_lib(dlFileName);
			if (handle == nullptr) {
				char const* const dlopen_error = dlerror();
				log->error(ERR_DLHANDLENOTFOUND, "File ", dlFileName, " found, but handle is null. Error: ",
				           dlopen_error);
			}
			else { log->info(0, "Loaded ", dlFileName); }
		}
		else { log->error(ERR_DLNOTFOUND, "could not find ", dlFileName); }
	}

	dlhandle handle = nullptr; // posix handle of the dynamic library

	~DynamicLib() {
		if (handle != nullptr) {
			close_lib(handle);
			log->debug(DESTRUCTOR, "Destroying ", dlFileName);
		}
	}


};


dlhandle load_lib(const std::string& lib) // never throws
{
	dlhandle h = nullptr;

	// If the caller already supplied a path (has a slash) just try it.
	if (lib.find('/') != std::string::npos) { h = dlopen(lib.c_str(), RTLD_NOW); }
	else {
		// 1. Try the file in the current working directory
		std::string cwdPath = "./" + lib;
		h                   = dlopen(cwdPath.c_str(), RTLD_NOW);

		// 2. Fallback to the normal search path so LD_LIBRARY_PATH,
		//    RPATH/RUNPATH, system dirs, etc. are still honoured.
		if (!h) { h = dlopen(lib.c_str(), RTLD_NOW); }
	}
	return h; // may be nullptr – caller should check and use dlerror()
}

static void close_lib(dlhandle handle) { dlclose(handle); }
