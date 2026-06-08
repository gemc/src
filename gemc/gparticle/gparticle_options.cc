// gparticle
#include "gparticle_options.h"
#include "gparticleConventions.h"
#include "gparticle_reader.h"

// gemc
#include "gfactory_options.h"
#include "gutilities.h"

// namespace to define options
namespace gparticle {
using std::string;
using std::vector;

// Reads a YAML scalar as a G4-unit-converted double.
//
// Accepted forms:
//   "20*deg"  — explicit unit, passed directly to getG4Number().
//   "20"      — plain number, falls back to fallback_unit with a logged warning.
//               Default units: angles → deg, momentum → MeV, lengths → cm.
//
// Returns default_g4val when the key is absent from the node (e.g. optional
// fields whose schema entry uses NODFLT or whose default was already merged).
static double parseG4Value(const YAML::Node&               node,
                           const string&                   key,
                           const string&                   fallback_unit,
                           double                          default_g4val,
                           const std::shared_ptr<GLogger>& logger) {
	if (!node[key]) return default_g4val;

	const auto val = node[key].as<string>();
	if (val.find('*') != string::npos) {
		return gutilities::getG4Number(val);
	}

	// Plain number without a unit — apply the fallback and warn.
	logger->warning("gparticle field <", key, "> value <", val, "> has no unit; assuming ",
	                fallback_unit, ". Use '", val, "*", fallback_unit, "' to suppress this warning.");
	return gutilities::getG4Number(val + "*" + fallback_unit);
}

// Build a vector of configured Gparticle instances from the structured "gparticle" option node.
// The detailed API contract is documented in gparticle_options.h.
vector<GparticlePtr> getGParticlesFromOption(const std::shared_ptr<GOptions>& gopts, std::shared_ptr<GLogger>& logger) {
	// Retrieve the structured option node that contains an array of particle definitions.
	auto gparticle_node = gopts->getOptionNode("gparticle");

	vector<GparticlePtr> gparticles;
	gparticles.reserve(gparticle_node.size());

	for (auto gparticle_item : gparticle_node) {
		gparticles.emplace_back(std::make_shared<Gparticle>(
			gopts->get_variable_in_option<string>(gparticle_item, "name", goptions::NODFLT),
			gopts->get_variable_in_option<int>(gparticle_item, "multiplicity", 1),

			parseG4Value(gparticle_item, "p",       "MeV", GPARTICLENOTDEFINED, logger),
			parseG4Value(gparticle_item, "delta_p", "MeV", 0,                   logger),
			gopts->get_variable_in_option<string>(gparticle_item, "randomMomentumModel", "uniform"),

			parseG4Value(gparticle_item, "theta",       "deg", 0, logger),
			parseG4Value(gparticle_item, "delta_theta", "deg", 0, logger),
			gopts->get_variable_in_option<string>(gparticle_item, "randomThetaModel", "uniform"),
			parseG4Value(gparticle_item, "phi",         "deg", 0, logger),
			parseG4Value(gparticle_item, "delta_phi",   "deg", 0, logger),

			parseG4Value(gparticle_item, "vx",       "cm", 0, logger),
			parseG4Value(gparticle_item, "vy",       "cm", 0, logger),
			parseG4Value(gparticle_item, "vz",       "cm", 0, logger),
			parseG4Value(gparticle_item, "delta_vx", "cm", 0, logger),
			parseG4Value(gparticle_item, "delta_vy", "cm", 0, logger),
			parseG4Value(gparticle_item, "delta_vz", "cm", 0, logger),

			gopts->get_variable_in_option<string>(gparticle_item, "randomVertexModel", "uniform"),
			logger
		));
	}

	return gparticles;
}

vector<GparticlePtr> getGParticles(const std::shared_ptr<GOptions>& gopts, std::shared_ptr<GLogger>& logger) {
	auto gparticles       = getGParticlesFromOption(gopts, logger);
	auto source_particles = getGParticlesFromSources(gopts, logger);
	gparticles.insert(gparticles.end(), source_particles.begin(), source_particles.end());
	return gparticles;
}


// Define the gparticle option schema and its human-readable help text.
// The detailed API contract is documented in gparticle_options.h.
GOptions defineOptions() {
	GOptions goptions(GPARTICLE_LOGGER);

	string help = "Adds a particle to the event generator.\n\n";
	help        += "Kinematic values accept an explicit Geant4 unit (e.g. '4*GeV', '23*deg', '1*mm').\n";
	help        += "A plain number without a unit falls back to the field default: MeV for momentum,\n";
	help        += "deg for angles, cm for vertex coordinates. A warning is logged in that case.\n\n";
	help        += "Examples:\n";
	help        += " - 5 GeV electron along z:\n";
	help        += "   -gparticle=\"[{name: e-, p: 5*GeV}]\"\n\n";
	help        += " - one electron and two protons spread in theta:\n";
	help        += "   -gparticle=\"[{name: e-, p: 2300*MeV, theta: 23*deg},\n";
	help        += "                {name: proton, multiplicity: 2, p: 1200*MeV, theta: 14*deg, delta_theta: 10*deg}]\"\n";

	vector<GVariable> gparticle_v = {
		{"name", goptions::NODFLT, "Particle name (mandatory), e.g. \"proton\" or \"e-\""},
		{"multiplicity", 1, "How many copies of this particle will be generated in each event"},
		{"p",       goptions::NODFLT, "Particle momentum with unit, e.g. \"4*GeV\" or \"4000*MeV\". "
		                             "Plain number falls back to MeV."},
		{"delta_p", "0*MeV", "Particle momentum spread, centered on p (same unit convention as p)."},
		{"randomMomentumModel", "uniform", "Momentum randomization. 'gaussian' uses deltas as sigmas."},
		{"theta",       "0*deg", "Particle polar angle, e.g. \"23*deg\" or \"0.4*rad\". "
		                        "Plain number falls back to deg."},
		{"delta_theta", "0*deg", "Particle polar angle spread, centered on theta."},
		{"randomThetaModel", "uniform",
		 "Distribute cos(theta) or theta. 'cosine': cos(theta) is uniform. 'uniform': theta is uniform."},
		{"phi",       "0*deg", "Particle azimuthal angle, e.g. \"90*deg\". Plain number falls back to deg."},
		{"delta_phi", "0*deg", "Particle azimuthal angle spread, centered on phi."},
		{"vx", "0*cm", "Particle vertex x component, e.g. \"1*mm\". Plain number falls back to cm."},
		{"vy", "0*cm", "Particle vertex y component."},
		{"vz", "0*cm", "Particle vertex z component."},
		{"delta_vx", "0*cm", "Particle vertex spread in x."},
		{"delta_vy", "0*cm", "Particle vertex spread in y."},
		{"delta_vz", "0*cm", "Particle vertex spread in z."},
		{"randomVertexModel", "uniform",
		 "Vertex randomization. 'uniform': flat. 'gaussian': deltas are sigmas. 'sphere': uniform in sphere."}
	};

	goptions.defineOption("gparticle", "define the generator particle(s)", gparticle_v, help);

	string file_help = "Adds particles to the event generator from particle-definition files. \n";
	file_help        += "The option is cumulative and each entry selects a reader by format and filename. \n \n";
	file_help        += "Built-in formats: \n";
	for (const auto& format : supported_static_reader_formats()) { file_help += "  - " + format + "\n"; }
	file_help += "\n";
	file_help += "Formats are case-insensitive. Additional formats can be provided by dynamic plugins named ";
	file_help += "gparticle_<format>_plugin.gplugin exporting GParticleReaderFactory. \n \n";
	file_help += "Example: \n";
	file_help += "-gparticlefile=\"[{format: lund, filename: a.lund}]\" \n";

	vector<GVariable> gparticlefile_v = {
		{"format", goptions::NODFLT, "Particle file format, for example \"lund\""},
		{"filename", goptions::NODFLT, "Input filename containing particle definitions"}
	};

	goptions.defineOption("gparticlefile", "define generator particles from file(s)", gparticlefile_v, file_help);

	goptions += gfactory::defineOptions();

	return goptions;
}
} // namespace gparticle
