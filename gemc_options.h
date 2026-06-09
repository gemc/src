#pragma once

// gemc
#include "goptions.h"

constexpr const char* GENERAL_LOGGER = "gemc";

namespace gemc {

    GOptions defineOptions();

    /**
     * \brief Scans YAML files in argv for gsystem names and probes each corresponding
     *        .gplugin for an optional \c definePluginOptions symbol.
     *
     * Called before the main GOptions parsing constructor so that plugin-specific
     * options are registered in the schema and appear in --help, saved config, and
     * command-line parsing alongside core GEMC options.
     *
     * The search path for plugins is assembled from (in priority order):
     * -# \c -plugin_path=... command-line arguments found in argv
     * -# \c plugin_path: entries found in YAML files listed in argv
     * -# the \c GEMC_PLUGIN_PATH environment variable
     * -# the current working directory
     *
     * Failures (missing plugin, missing symbol, bad YAML) are silently skipped;
     * not every system has a plugin and not every plugin declares custom options.
     *
     * \param argc  Argument count from \c main().
     * \param argv  Argument vector from \c main().
     * Plugin contract: the symbol must be declared as
     * \code{.cpp}
     *   extern "C" GOptions* definePluginOptions();
     * \endcode
     * The returned pointer is heap-allocated and owned by the caller (deleted immediately
     * after merging). Returning a pointer rather than a value avoids the C-linkage /
     * non-trivial-return-type warning that clang emits for by-value C++ returns across
     * an \c extern "C" boundary.
     *
     * \return A \c GOptions containing only the option schema contributions from plugins.
     *         Values are not parsed yet; that happens in the main constructor.
     */
    GOptions collectPluginOptions(int argc, char* argv[]);

}


