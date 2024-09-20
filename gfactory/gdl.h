#ifndef  GDYNAMICLIB_H
#define  GDYNAMICLIB_H  1

// documentation: http://www.faqs.org/docs/Linux-mini/C++-dlopen.html

// glibrary: for color logs definitions
#include "gutsConventions.h"

// c++ plugin loading functions
#include <dlfcn.h>

// c++
#include <sys/stat.h> // to check if file exists
#include <string>
#include <iostream>

typedef void *dlhandle;

static dlhandle load_lib(const std::string &path);

static void close_lib(dlhandle handle);

#define PLUGINITEM   " ⎆"

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
    int verbosity = 0;

    bool doesFileExists(const std::string &name) {
        struct stat buffer;
        return (stat(name.c_str(), &buffer) == 0);
    }


public:
    // default constructor
    DynamicLib() = default;

    dlhandle handle = nullptr;   // posix handle of the dynamic library

    DynamicLib(std::string path, int v = 0) : dlFileName(path), verbosity(v), handle(nullptr) {

        if (verbosity > 0) {
            std::cout << PLUGINITEM << " Trying (1) " << YELLOWHHL << dlFileName << RSTHHR << std::endl;
        }

        // trying $GEMC/lib/ + path if not found
        if (!doesFileExists(dlFileName)) {
            if (verbosity > 0) {
                std::cout << PLUGINITEM << " Trying (2) " << YELLOWHHL << dlFileName << RSTHHR << std::endl;
            }
            dlFileName = std::string(getenv("GEMC")) + "/lib/" + path;
        }

        // trying $GEMC/lib64/ + path if not found
        if (!doesFileExists(dlFileName)) {
            if (verbosity > 0) {
                std::cout << PLUGINITEM << " Trying (3) " << YELLOWHHL << dlFileName << RSTHHR << std::endl;
            }
            dlFileName = std::string(getenv("GEMC")) + "/lib64/" + path;
        }

        if (doesFileExists(dlFileName)) {
            handle = load_lib(dlFileName);
            if (handle == nullptr) {
                char const *const dlopen_error = dlerror();

                std::cerr << FATALERRORL << "File " << YELLOWHHL << dlFileName << RSTHHR << " found, but handle dlopenened is null" << std::endl;
                std::cerr << "dlopen error: " << dlopen_error << std::endl;
                gexit(EC__DLHANDLENOTFOUND);
            }

        } else {
            std::cerr << FATALERRORL << "could not load " << YELLOWHHL << dlFileName << RSTHHR << std::endl;
            gexit(EC__DLNOTFOUND);
        }
    }

    ~DynamicLib() {
        if (handle != nullptr) {
            close_lib(handle);
            if (verbosity > 1) {
                std::cout << PLUGINITEM << " Closing DL " << YELLOWHHL << dlFileName << RSTHHR << std::endl;
            }
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
