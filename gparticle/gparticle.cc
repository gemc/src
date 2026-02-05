// guts
#include "gutilities.h"

// gparticle
#include "gparticle.h"
#include "gparticle_options.h"

// gemc
#include "gparticleConventions.h"

// geant4
#include "G4ParticleTable.hh"
#include "G4Event.hh"
#include "Randomize.hh"

using std::to_string;
using std::ostream;
using std::endl;
using std::string;

// Constructor based on parameters.
// Detailed API documentation is in gparticle.h.
Gparticle::Gparticle(const std::string&              aname,
                     int                             amultiplicity,
                     double                          ap,
                     double                          adelta_p,
                     const std::string&              punit,
                     const std::string&              arandomMomentumModel,
                     double                          atheta,
                     double                          adelta_theta,
                     const std::string&              arandomThetaModel,
                     double                          aphi,
                     double                          adelta_phi,
                     const std::string&              aunit,
                     double                          avx,
                     double                          avy,
                     double                          avz,
                     double                          adelta_vx,
                     double                          adelta_vy,
                     double                          adelta_vz,
                     const std::string&              vunit,
                     const std::string&              arandomVertexModel,
                     const std::shared_ptr<GLogger>& logger) :
	name(aname),
	multiplicity(amultiplicity),
	// Convert user values + unit strings into Geant4 numeric values through gutilities.
	p(gutilities::getG4Number(to_string(ap) + "*" + punit)),
	delta_p(gutilities::getG4Number(to_string(adelta_p) + "*" + punit)),
	randomMomentumModel(gutilities::stringToRandomModel(arandomMomentumModel)),
	theta(gutilities::getG4Number(to_string(atheta) + "*" + aunit)),
	delta_theta(gutilities::getG4Number(to_string(adelta_theta) + "*" + aunit)),
	randomThetaModel(gutilities::stringToRandomModel(arandomThetaModel)),
	phi(gutilities::getG4Number(to_string(aphi) + "*" + aunit)),
	delta_phi(gutilities::getG4Number(to_string(adelta_phi) + "*" + aunit)),
	v(G4ThreeVector(
		gutilities::getG4Number(to_string(avx) + "*" + vunit),
		gutilities::getG4Number(to_string(avy) + "*" + vunit),
		gutilities::getG4Number(to_string(avz) + "*" + vunit)
	)),
	delta_v(G4ThreeVector(
		gutilities::getG4Number(to_string(adelta_vx) + "*" + vunit),
		gutilities::getG4Number(to_string(adelta_vy) + "*" + vunit),
		gutilities::getG4Number(to_string(adelta_vz) + "*" + vunit)
	)),
	randomVertexModel(gutilities::stringToRandomModel(arandomVertexModel)),
	log(logger) {
	// Resolve PDG id immediately so errors are detected early and configuration printing is complete.
	pid = get_pdg_id();

	log->debug(CONSTRUCTOR, "Gparticle");

	// Print a full configuration summary at verbosity level 2.
	log->info(2, *this);
}


// Shoots this particle into the provided event using the provided Geant4 particle gun.
// Detailed API documentation is in gparticle.h.
void Gparticle::shootParticle(G4ParticleGun* particleGun, G4Event* anEvent) {
	auto particleTable = G4ParticleTable::GetParticleTable();

	if (particleTable) {
		// Resolve the particle definition by name.
		auto particleDef = particleTable->FindParticle(name);

		if (particleDef) {
			// Mass is used to convert randomized momentum magnitude into kinetic energy.
			double mass = particleDef->GetPDGMass();
			particleGun->SetParticleDefinition(particleDef);

			// Shoot one primary vertex per multiplicity.
			for (int i = 0; i < multiplicity; i++) {
				particleGun->SetParticleEnergy(calculateKinEnergy(mass));
				particleGun->SetParticleMomentumDirection(calculateBeamDirection());
				particleGun->SetParticlePosition(calculateVertex());
				particleGun->GeneratePrimaryVertex(anEvent);
			}
		}
		else {
			log->error(ERR_GPARTICLENOTFOUND,
			           "Particle <", name, "> not found in G4ParticleTable* ", particleTable);
		}
	}
	else {
		log->error(ERR_GPARTICLETABLENOTFOUND,
		           "G4ParticleTable not found - G4ParticleGun*: ", particleGun);
	}
}


double Gparticle::calculateMomentum() {
	// randomizeNumberFromSigmaWithModel applies the model-dependent interpretation of delta.
	double pmev = randomizeNumberFromSigmaWithModel(p, delta_p, randomMomentumModel);

	return pmev;
}

double Gparticle::calculateKinEnergy(double mass) {
	double pmev = calculateMomentum();

	return sqrt(pmev * pmev + mass * mass) - mass;
}


G4ThreeVector Gparticle::calculateBeamDirection() {
	// Convert to radians for trigonometric functions.
	double thetaRad = randomizeNumberFromSigmaWithModel(theta, delta_theta, randomThetaModel) / CLHEP::rad;
	double phiRad   = randomizeNumberFromSigmaWithModel(phi, delta_phi, gutilities::uniform) / CLHEP::rad;

	G4ThreeVector pdir = G4ThreeVector(
		cos(phiRad) * sin(thetaRad),
		sin(phiRad) * sin(thetaRad),
		cos(thetaRad)
	);

	return pdir;
}

G4ThreeVector Gparticle::calculateVertex() {
	double x, y, z;

	switch (randomVertexModel) {
	case gutilities::uniform:
		// Component-wise uniform sampling around the nominal vertex.
		x = randomizeNumberFromSigmaWithModel(v.x(), delta_v.x(), gutilities::uniform);
		y = randomizeNumberFromSigmaWithModel(v.y(), delta_v.y(), gutilities::uniform);
		z = randomizeNumberFromSigmaWithModel(v.z(), delta_v.z(), gutilities::uniform);
		break;

	case gutilities::gaussian:
		// Component-wise Gaussian sampling around the nominal vertex (deltas used as sigmas).
		x = randomizeNumberFromSigmaWithModel(v.x(), delta_v.x(), gutilities::gaussian);
		y = randomizeNumberFromSigmaWithModel(v.y(), delta_v.y(), gutilities::gaussian);
		z = randomizeNumberFromSigmaWithModel(v.z(), delta_v.z(), gutilities::gaussian);
		break;

	case gutilities::sphere: {
		// Sample an offset inside a sphere-like region whose maximum radius is delta_v.r().
		// Assumes all three components have comparable spread such that delta_v.r() is meaningful.
		double radius;
		double max_radius = delta_v.r();

		// Rejection sampling: generate a random point in a cube until it lies within the radius bound.
		do {
			x      = randomizeNumberFromSigmaWithModel(0, max_radius, gutilities::uniform);
			y      = randomizeNumberFromSigmaWithModel(0, max_radius, gutilities::uniform);
			z      = randomizeNumberFromSigmaWithModel(0, max_radius, gutilities::uniform);
			radius = x * x + y * y + z * z;
		}
		while (radius > max_radius);

		// Offset the sampled point by the nominal vertex.
		x = x + v.x();
		y = y + v.y();
		z = z + v.z();
		break;
	}

	default:
		// Unknown model: fall back to deterministic vertex.
		x = v.x();
		y = v.y();
		z = v.z();
		break;
	}

	return {x, y, z};
}


double Gparticle::randomizeNumberFromSigmaWithModel(double center, double delta, gutilities::randomModel model) const {
	switch (model) {
	case gutilities::uniform:
		// Uniform in [center-delta, center+delta].
		return center + (2.0 * G4UniformRand() - 1.0) * delta;

	case gutilities::gaussian:
		// Gaussian with mean=center and sigma=delta.
		return G4RandGauss::shoot(center, delta);

	case gutilities::cosine: {
		// assuming this is an angle with corrected units
		// For cosine-weighted sampling we work in radians, sample theta with sin(theta) weighting,
		// and then enforce the requested [center-delta, center+delta] range.
		double lower      = (center - delta) / CLHEP::rad;
		double upper      = (center + delta) / CLHEP::rad;
		double center_rad = 0;

		if (lower < upper) {
			// Generate theta such that cos(theta) is uniform, which corresponds to sin(theta) weighting.
			do { center_rad = acos(1 - 2 * G4UniformRand()); }
			while (center_rad < lower || center_rad > upper);
		}
		else {
			// Degenerate range: fall back to the stored theta value.
			center_rad = theta / CLHEP::rad;
		}

		return center_rad * CLHEP::rad;
	}

	default:
		// Unknown model: no randomization.
		return center;
	}
}

// ---------------------------------------------------------------------------
//  pretty printer
// ---------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const Gparticle& gp) {
	using std::left;
	using std::right;
	using std::setw;

	constexpr int label_w = 15; // width for the field name (with ':')
	constexpr int value_w = 12; // width for the main column

	// helper: plain value
	auto show = [&](const std::string& label, const auto& value) {
		os << left << setw(label_w) << label << ' '
			<< right << setw(value_w) << value << '\n';
	};

	// helper: double value with N decimals
	auto showf = [&](const std::string& label, double value, int prec = 3) {
		std::streamsize old_prec  = os.precision(); // save current
		auto            old_flags = os.flags();     // save flags

		os << left << setw(label_w) << label << ' '
			<< right << setw(value_w) << std::fixed
			<< std::setprecision(prec) << value << '\n';

		os.precision(old_prec); // restore
		os.flags(old_flags);
	};

	// helper: value ± error (both doubles)
	auto show_pm = [&](const std::string& label,
	                   double             val, double err,
	                   int                prec = 3) {
		std::streamsize old_prec  = os.precision();
		auto            old_flags = os.flags();

		os << left << setw(label_w) << label << ' '
			<< right << setw(value_w) << std::fixed
			<< std::setprecision(prec) << val
			<< "  ± " << std::setprecision(prec) << err << '\n';

		os.precision(old_prec);
		os.flags(old_flags);
	};

	// -----------------------------------------------------------------------
	//  header block
	// -----------------------------------------------------------------------
	os << '\n'
		<< " ┌─────────────────────────────────────────────────┐\n"
		<< " │ GParticle                                       │\n"
		<< " └─────────────────────────────────────────────────┘\n";

	// -----------------------------------------------------------------------
	//  fields
	// -----------------------------------------------------------------------
	os << left << setw(label_w) << " name:" << right << setw(value_w)
		<< gp.name << "(pid " << gp.pid << ")\n";

	show(" multiplicity:", std::to_string(gp.multiplicity));
	showf(" mass [MeV]:", gp.get_mass());

	show_pm(" p [MeV]:",
	        gp.p,
	        gp.delta_p / CLHEP::MeV);

	show(" p model:", to_string(gp.randomMomentumModel));

	show_pm(" theta [deg]:",
	        gp.theta / CLHEP::deg,
	        gp.delta_theta / CLHEP::deg);

	show(" theta model:", to_string(gp.randomThetaModel));

	show_pm(" phi  [deg]:",
	        gp.phi / CLHEP::deg,
	        gp.delta_phi / CLHEP::deg);

	os << left << setw(label_w) << " vertex [cm]:" << ' '
		<< gp.v << "  ± " << gp.delta_v << '\n';

	show(" vertex model:", to_string(gp.randomVertexModel));

	return os;
}


int Gparticle::get_pdg_id() {
	auto particleTable = G4ParticleTable::GetParticleTable();

	if (particleTable) {
		auto particleDef = particleTable->FindParticle(name);

		if (particleDef != nullptr) { return particleDef->GetPDGEncoding(); }
		else {
			log->error(ERR_GPARTICLENOTFOUND,
			           "Particle <", name, "> not found in G4ParticleTable* ", particleTable);
		}
	}
	else {
		log->error(ERR_GPARTICLETABLENOTFOUND,
		           "G4ParticleTable not found - G4ParticleGun*: ", particleTable);
	}
}


double Gparticle::get_mass() const {
	auto particleTable = G4ParticleTable::GetParticleTable();

	if (particleTable) {
		auto particleDef = particleTable->FindParticle(name);

		if (particleDef) {
			double mass = particleDef->GetPDGMass();
			return mass;
		}
	}
	return 0;
}
