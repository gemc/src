#pragma once

// gemc
#include "goptions.h"
#include "gsystem.h"
#include "gmodifier.h"

/**
 * \file gsystem_options.h
 * \brief Option definitions and extraction helpers for the gsystem module.
 *
 * The gsystem module reads detector-system configuration from GOptions and provides:
 * - construction of a list of GSystem descriptors (\ref gsystem::getSystems "getSystems()");
 * - construction of a list of post-load modifiers (\ref gsystem::getModifiers "getModifiers()");
 * - definition of module options (\ref gsystem::defineOptions "defineOptions()").
 */

constexpr const char* GVOLUME_LOGGER = "gvolume";
constexpr const char* GMATERIAL_LOGGER = "gmaterial";
constexpr const char* GSYSTEM_LOGGER = "gsystem";
constexpr const char* GWORLD_LOGGER = "gworld";
constexpr const char* GSFACTORY_LOGGER = "gsfactory";

namespace gsystem {

/**
 * \brief Build a list of systems from options.
 *
 * \param gopts Shared options/configuration instance.
 * \return List of system descriptors to be loaded into a world.
 *
 * \details This reads the "gsystem" option node. For each entry it extracts:
 * - name (mandatory)
 * - factory (default: sqlite)
 * - variation (default: "default")
 * - annotations (default: UNINITIALIZEDSTRINGQUANTITY)
 */
SystemList getSystems(const std::shared_ptr<GOptions>& gopts);

/**
 * \brief Build a list of volume modifiers from options.
 *
 * \param gopts Shared options/configuration instance.
 * \return List of modifiers to apply during world construction.
 *
 * \details This reads the "gmodifier" option node and converts each entry into a GModifier.
 */
std::vector<GModifier> getModifiers(const std::shared_ptr<GOptions>& gopts);

/**
 * \brief Define and return all option groups required by the gsystem module.
 *
 * \return A fully populated GOptions instance containing logger groups and module-specific options.
 */
GOptions defineOptions();

}
