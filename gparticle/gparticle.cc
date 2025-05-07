// guts
#include "gutilities.h"

using namespace gutilities;

// gparticle
#include "gparticle.h"
#include "gparticleConventions.h"

// geant4
#include "G4ParticleTable.hh"
#include "G4Event.hh"
#include "Randomize.hh"

// c++
using namespace std;


// Constructor based on parameters
Gparticle::Gparticle(string aname,
                     int    amultiplicity,
                     float  ap,
                     float  adelta_p,
                     string punit,
                     string arandomMomentumModel,

                     float  atheta,
                     float  adelta_theta,
                     string arandomThetaModel,
                     float  aphi,
                     float  adelta_phi,
                     string aunit,

                     float                    avx,
                     float                    avy,
                     float                    avz,
                     float                    adelta_vx,
                     float                    adelta_vy,
                     float                    adelta_vz,
                     string                   vunit,
                     string                   arandomVertexModel,
                     std::shared_ptr<GLogger> log):
	name(aname),
	multiplicity(amultiplicity),
	p(getG4Number(to_string(ap) + "*" + punit)),
	delta_p(getG4Number(to_string(adelta_p) + "*" + punit)),
	randomMomentumModel(gutilities::stringToRandomModel(arandomMomentumModel)),
	theta(getG4Number(to_string(atheta) + "*" + aunit)),
	delta_theta(getG4Number(to_string(adelta_theta) + "*" + aunit)),
	randomThetaModel(gutilities::stringToRandomModel(arandomThetaModel)),
	phi(getG4Number(to_string(aphi) + "*" + aunit)),
	delta_phi(getG4Number(to_string(adelta_phi) + "*" + aunit)),
	v(G4ThreeVector(
	                getG4Number(to_string(avx) + "*" + vunit),
	                getG4Number(to_string(avy) + "*" + vunit),
	                getG4Number(to_string(avz) + "*" + vunit)
	               )),
	delta_v(G4ThreeVector(
	                      getG4Number(to_string(adelta_vx) + "*" + vunit),
	                      getG4Number(to_string(adelta_vy) + "*" + vunit),
	                      getG4Number(to_string(adelta_vz) + "*" + vunit)
	                     )),
	randomVertexModel(gutilities::stringToRandomModel(arandomVertexModel)) {
	pid = get_pdg_id(log);

	// print particle with ostream operator
	log->info(2, this);
}


// for G4GeneralParticleSource API check:
// https://geant4.kek.jp/lxr/source/event/include/G4ParticleGun.hh
// https://geant4.kek.jp/lxr/source/event/src/G4ParticleGun.cc
void Gparticle::shootParticle(G4ParticleGun* particleGun, G4Event* anEvent, std::shared_ptr<GLogger> log) {
	auto particleTable = G4ParticleTable::GetParticleTable();

	if (particleTable) {
		auto particleDef = particleTable->FindParticle(name);

		if (particleDef) {
			float mass = particleDef->GetPDGMass();
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


float Gparticle::calculateMomentum() {
	float pmev = randomizeNumberFromSigmaWithModel(p, delta_p, randomMomentumModel);

	return pmev;
}

float Gparticle::calculateKinEnergy(float mass) {
	float pmev = calculateMomentum();

	return sqrt(pmev * pmev + mass * mass) - mass;
}


G4ThreeVector Gparticle::calculateBeamDirection() {
	float thetaRad = randomizeNumberFromSigmaWithModel(theta, delta_theta, randomThetaModel) / CLHEP::rad;

	float phiRad = randomizeNumberFromSigmaWithModel(phi, delta_phi, uniform) / CLHEP::rad;

	G4ThreeVector pdir = G4ThreeVector(
	                                   cos(phiRad) * sin(thetaRad),
	                                   sin(phiRad) * sin(thetaRad),
	                                   cos(thetaRad)
	                                  );

	return pdir;
}

G4ThreeVector Gparticle::calculateVertex() {
	float x, y, z;

	switch (randomVertexModel) {
	case uniform:
		x = randomizeNumberFromSigmaWithModel(v.x(), delta_v.x(), uniform);
		y = randomizeNumberFromSigmaWithModel(v.y(), delta_v.y(), uniform);
		z = randomizeNumberFromSigmaWithModel(v.z(), delta_v.z(), uniform);
		break;
	case gaussian:
		x = randomizeNumberFromSigmaWithModel(v.x(), delta_v.x(), gaussian);
		y = randomizeNumberFromSigmaWithModel(v.y(), delta_v.y(), gaussian);
		z = randomizeNumberFromSigmaWithModel(v.z(), delta_v.z(), gaussian);
		break;
	case sphere: {
		float radius;
		float max_radius = delta_v.r();
		// assumes all 3 components have the same spread
		do {
			x      = randomizeNumberFromSigmaWithModel(0, max_radius, uniform);
			y      = randomizeNumberFromSigmaWithModel(0, max_radius, uniform);
			z      = randomizeNumberFromSigmaWithModel(0, max_radius, uniform);
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

	return G4ThreeVector(x, y, z);
}


float Gparticle::randomizeNumberFromSigmaWithModel(float center, float delta, gutilities::randomModel model) {
	switch (model) {
	case uniform:
		return center + (2.0 * G4UniformRand() - 1.0) * delta;

	case gaussian:
		return G4RandGauss::shoot(center, delta);

	case cosine: { // assuming this is an angle with corrected units
		float lower      = (center - delta) / CLHEP::rad;
		float upper      = (center + delta) / CLHEP::rad;
		float center_rad = center / CLHEP::rad;

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


int Gparticle::get_pdg_id(shared_ptr<GLogger> log) {
	auto particleTable = G4ParticleTable::GetParticleTable();

	if (particleTable) {
		auto particleDef = particleTable->FindParticle(name);

		if (particleDef) {
			return particleDef->GetPDGEncoding();
		} else {
			log->error(ERR_GPARTICLENOTFOUND,
			           "Particle <", name, "> not found in G4ParticleTable* ", particleTable);
		}
	}
}


double Gparticle::get_mass() {
	auto particleTable = G4ParticleTable::GetParticleTable();

	if (particleTable) {
		auto particleDef = particleTable->FindParticle(name);

		if (particleDef) {
			float mass = particleDef->GetPDGMass();
			return mass;
		}
	}
	return 0;
}
