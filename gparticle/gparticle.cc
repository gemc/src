// guts
#include "gutilities.h"

// gparticle
#include "gparticle.h"

#include <utility>
#include "gparticleConventions.h"

// geant4
#include "G4ParticleTable.hh"
#include "G4Event.hh"
#include "Randomize.hh"

using std::to_string;
using std::ostream;
using std::endl;


// Constructor based on parameters
Gparticle::Gparticle(string                          aname,
                     int                             amultiplicity,
                     double                          ap,
                     double                          adelta_p,
                     const string&                   punit,
                     const string&                   arandomMomentumModel,
                     double                          atheta,
                     double                          adelta_theta,
                     const string&                   arandomThetaModel,
                     double                          aphi,
                     double                          adelta_phi,
                     const string&                   aunit,
                     double                          avx,
                     double                          avy,
                     double                          avz,
                     double                          adelta_vx,
                     double                          adelta_vy,
                     double                          adelta_vz,
                     const string&                   vunit,
                     const string&                   arandomVertexModel,
                     const std::shared_ptr<GLogger>& log):
	name(std::move(aname)),
	multiplicity(amultiplicity),
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
	randomVertexModel(gutilities::stringToRandomModel(arandomVertexModel)) {
	pid = get_pdg_id(log);

	// print particle with ostream operator
	log->info(2, this);
}


// for G4GeneralParticleSource API check:
// https://geant4.kek.jp/lxr/source/event/include/G4ParticleGun.hh
// https://geant4.kek.jp/lxr/source/event/src/G4ParticleGun.cc
void Gparticle::shootParticle(G4ParticleGun* particleGun, G4Event* anEvent, const std::shared_ptr<GLogger>& log) {

	auto particleTable = G4ParticleTable::GetParticleTable();

	if (particleTable) {

		auto particleDef = particleTable->FindParticle(name);

		if (particleDef) {
			double mass = particleDef->GetPDGMass();
			particleGun->SetParticleDefinition(particleDef);

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
	double pmev = randomizeNumberFromSigmaWithModel(p, delta_p, randomMomentumModel);

	return pmev;
}

double Gparticle::calculateKinEnergy(double mass) {
	double pmev = calculateMomentum();

	return sqrt(pmev * pmev + mass * mass) - mass;
}


G4ThreeVector Gparticle::calculateBeamDirection() {
	double thetaRad = randomizeNumberFromSigmaWithModel(theta, delta_theta, randomThetaModel) / CLHEP::rad;

	double phiRad = randomizeNumberFromSigmaWithModel(phi, delta_phi, gutilities::uniform) / CLHEP::rad;

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
		x = randomizeNumberFromSigmaWithModel(v.x(), delta_v.x(), gutilities::uniform);
		y = randomizeNumberFromSigmaWithModel(v.y(), delta_v.y(), gutilities::uniform);
		z = randomizeNumberFromSigmaWithModel(v.z(), delta_v.z(), gutilities::uniform);
		break;
	case gutilities::gaussian:
		x = randomizeNumberFromSigmaWithModel(v.x(), delta_v.x(), gutilities::gaussian);
		y = randomizeNumberFromSigmaWithModel(v.y(), delta_v.y(), gutilities::gaussian);
		z = randomizeNumberFromSigmaWithModel(v.z(), delta_v.z(), gutilities::gaussian);
		break;
	case gutilities::sphere: {
		double radius;
		double max_radius = delta_v.r();
		// assumes all 3 components have the same spread
		do {
			x      = randomizeNumberFromSigmaWithModel(0, max_radius, gutilities::uniform);
			y      = randomizeNumberFromSigmaWithModel(0, max_radius, gutilities::uniform);
			z      = randomizeNumberFromSigmaWithModel(0, max_radius, gutilities::uniform);
			radius = x * x + y * y + z * z;
		}
		while (radius > max_radius);
		x = x + v.x();
		y = y + v.y();
		z = z + v.z();
		break;
	}
	default:
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
		return center + (2.0 * G4UniformRand() - 1.0) * delta;

	case gutilities::gaussian:
		return G4RandGauss::shoot(center, delta);

	case gutilities::cosine: { // assuming this is an angle with corrected units
		double lower      = (center - delta) / CLHEP::rad;
		double upper      = (center + delta) / CLHEP::rad;
		double center_rad = center / CLHEP::rad;

		if (lower < upper) {
			// generate random cos(theta) in range [lower, upper]
			do { center_rad = acos(1 - 2 * G4UniformRand()); }
			while (center_rad < lower || center_rad > upper);
		}
		else { center_rad = theta / CLHEP::rad; }

		return center_rad * CLHEP::rad;
	}

	default:
		return center;
	}
}

ostream& operator<<(ostream& stream, Gparticle gparticle) {
	stream << "Gparticle: " << endl;

	stream << "  name: " << gparticle.name << " (pid: " << gparticle.pid << ")" << endl;
	stream << "  multiplicity: " << gparticle.multiplicity << endl;
	stream << "  mass: " << gparticle.get_mass() << endl;
	stream << "  p: " << gparticle.p << endl;
	stream << "  delta_p: " << gparticle.delta_p / CLHEP::MeV << " MeV" << endl;
	stream << "  randomMomentumModel: " << gparticle.randomMomentumModel << endl;
	stream << "  theta: " << gparticle.theta / CLHEP::deg << " deg" << endl;
	stream << "  delta_theta: " << gparticle.delta_theta / CLHEP::deg << " deg" << endl;
	stream << "  randomThetaModel: " << gparticle.randomThetaModel << endl;
	stream << "  phi: " << gparticle.phi / CLHEP::deg << "deg" << endl;
	stream << "  delta_phi: " << gparticle.delta_phi / CLHEP::deg << "deg" << endl;
	stream << "  v: " << gparticle.v << endl;
	stream << "  delta_v: " << gparticle.delta_v << endl;
	stream << "  randomVertexModel: " << gparticle.randomVertexModel << endl;

	return stream;
}


int Gparticle::get_pdg_id(const std::shared_ptr<GLogger>& log) {
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


double Gparticle::get_mass() {
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
