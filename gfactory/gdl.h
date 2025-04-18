#ifndef  GDYNAMICLIB_H
#define  GDYNAMICLIB_H  1

#include "glogger.h"

// c++ plugin loading functions
// documentation: http://www.faqs.org/docs/Linux-mini/C++-dlopen.html
#include <dlfcn.h>

// c++
#include <sys/stat.h> // to check if file exists
#include <string>
#include <iostream>

typedef void *dlhandle;

static dlhandle load_lib(const std::string &path);

static void close_lib(dlhandle handle);

// exit codes: 1000s
#define EC__DLNOTFOUND       1001
#define EC__FACTORYNOTFOUND  1002
#define EC__DLHANDLENOTFOUND 1003


/**
 * @struct DynamicLib
 * @brief  Structure to load dynamically symbols from a shared library
 */
struct DynamicLib {

private:
	std::string dlFileName;  // dynamic library file

	bool doesFileExists(const std::string &name) {
		struct stat buffer;
		return (stat(name.c_str(), &buffer) == 0);
	}

	std::shared_ptr<GLogger> log;

public:
	// default constructor
	DynamicLib() = default;

	// path here is the filename
	DynamicLib(std::shared_ptr<GLogger> logger, std::string path) : dlFileName(path), log(std::move(logger)), handle(nullptr) {

		log->debug(CONSTRUCTOR, "Instantiating ", path);
		log->debug(NORMAL, "Trying ", dlFileName);

		// trying $GEMC/lib/ + path if not found
		if (!doesFileExists(dlFileName)) {
			log->debug(NORMAL, dlFileName, "Not found...");
			dlFileName = std::string(getenv("GEMC")) + "/lib/" + path;
			log->debug(NORMAL, "Trying ", dlFileName);
		}

		// trying $GEMC/lib64/ + path if not found
		if (!doesFileExists(dlFileName)) {
			log->debug(NORMAL, dlFileName, "Not found...");
			dlFileName = std::string(getenv("GEMC")) + "/lib64/" + path;
			log->debug(NORMAL, "Trying ", dlFileName);
		}

		if (doesFileExists(dlFileName)) {
			handle = load_lib(dlFileName);
			log->info(0, "Loaded ", dlFileName);
			if (handle == nullptr) {
				char const *const dlopen_error = dlerror();
				log->error(EC__DLHANDLENOTFOUND, "File ", dlFileName, " found, but handle is null. Error: ",
						   dlopen_error);
			}
		} else {
			log->error(EC__DLNOTFOUND, "could not find ", dlFileName);
		}
	}

	dlhandle handle = nullptr;   // posix handle of the dynamic library

	~DynamicLib() {
		if (handle != nullptr) {
			close_lib(handle);
			log->debug(DESTRUCTOR, "Destroying ", dlFileName);
		}
	}


};


static dlhandle load_lib(const std::string &path) {

	return dlopen(path.data(), RTLD_NOW);
	// get a handle to the lib, may be nullptr.
	// RTLD_NOW ensures that all the symbols are resolved immediately:
	// if a symbol cannot be found, the program will crash now instead of later.
}

static void close_lib(dlhandle handle) {
	dlclose(handle);
}

#endif
