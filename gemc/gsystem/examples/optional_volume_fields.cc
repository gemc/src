/**
 * \file optional_volume_fields.cc
 * \brief Verifies normalization of optional serialized geometry fields.
 */

#include "gvolume.h"

#include <cstdlib>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace {

std::vector<std::string> volume_parameters(const std::string& emfield,
                                           const std::string& digitization,
                                           const std::string& identity,
                                           const std::string& copy_of,
                                           const std::string& solids_operation,
	                                       const std::string& mirror,
	                                       const std::string& parameters = "1*cm, 2*cm, 3*cm") {
	return {
		"test_volume", "G4Box", parameters, "G4_AIR", "root",
		"0*cm, 0*cm, 0*cm", "0*deg, 0*deg, 0*deg", "active", emfield,
		"1", "1", "778899", "1", digitization, identity, copy_of,
		solids_operation, mirror, "1", "optional field test", "default", "1"
	};
}

} // namespace

int main() {
	const std::shared_ptr<GLogger> logger;
	GVolume unset(logger, "test",
	              volume_parameters(" NULL ", " not provided ", "~", " no ", "", " none "),
	              " none ");

	if (unset.getEMField() || unset.getDigitization() || unset.getGIdentity() || unset.getCopyOf() ||
	    unset.getSolidsOpr() || unset.getMirror() || unset.getImportedFile()) {
		return EXIT_FAILURE;
	}
	GVolume imported(logger, "test",
	                 volume_parameters("NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL"));
	if (imported.getParameters() || !imported.getDetectorDimensions().empty()) return EXIT_FAILURE;

	GVolume configured(logger, "test",
	                   volume_parameters(" torus field ", " flux ", " sector: 1 ", " source ",
	                                     " left + right ", " aluminum "),
	                   " meshes/part.stl ");

	if (configured.getEMField() != std::optional<std::string>{"torusfield"} ||
	    configured.getDigitization() != std::optional<std::string>{"flux"} ||
	    configured.getGIdentity() != std::optional<std::string>{"sector: 1"} ||
	    configured.getCopyOf() != std::optional<std::string>{"source"} ||
	    configured.getSolidsOpr() != std::optional<std::string>{"left + right"} ||
	    configured.getMirror() != std::optional<std::string>{"aluminum"} ||
	    configured.getImportedFile() != std::optional<std::string>{"meshes/part.stl"}) {
		return EXIT_FAILURE;
	}

	configured.setDigitization(" NULL ");
	configured.setGIdentity(std::nullopt);
	return configured.getDigitization() || configured.getGIdentity() ? EXIT_FAILURE : EXIT_SUCCESS;
}
