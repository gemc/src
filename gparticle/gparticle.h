#pragma once

// gemc
#include "glogger.h"
#include "gutilities.h"

// geant4
#include "G4ThreeVector.hh"
#include "G4ParticleGun.hh"

// c++
#include <string>


// This class interfaces to the Geant4 General Particle Source
class Gparticle {

public:
	// Constructor based on parameters
	Gparticle(std::string               name,
	          int                       multiplicity,
	          double                    p,
	          double                    delta_p,
	          const std::string&        punit,
	          const std::string&        randomMomentumModel,
	          double                    theta,
	          double                    delta_theta,
	          const std::string&        thetaModel,
	          double                    phi,
	          double                    delta_phi,
	          const std::string&        aunit,
	          double                    avx,
	          double                    avy,
	          double                    avz,
	          double                    adelta_vx,
	          double                    adelta_vy,
	          double                    adelta_vz,
	          const std::string&        vunit,
	          const std::string&        randomVertexModel,
	          std::shared_ptr<GLogger>& logger);

	// explicitly delete the copy constructor/ assignment,
	// we only move unique pointers to Gparticles
	Gparticle(const Gparticle&)            = delete;
	Gparticle& operator=(const Gparticle&) = delete;

	~Gparticle() { log->debug(DESTRUCTOR, "Gparticle"); }

private:
	// PDG Monte Carlo Particle Numbering Scheme:
	// https://pdg.lbl.gov/ > Reviews Tables and Plots > Mathematical Tools > Numbering Scheme
	std::string name;
	int         pid;
	int         multiplicity;

	// assigned momentum and distributions. See particle options for description
	double                  p;
	double                  delta_p;
	gutilities::randomModel randomMomentumModel;

	double                  theta;
	double                  delta_theta;
	gutilities::randomModel randomThetaModel;

	double phi;
	double delta_phi;

	G4ThreeVector           v;
	G4ThreeVector           delta_v;
	gutilities::randomModel randomVertexModel;


	[[nodiscard]] double get_mass() const;

	friend std::ostream& operator<<(std::ostream& os, const Gparticle& gp);

	friend std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Gparticle>& ptr) {
		if (ptr) return os << *ptr;
		else return os << "<null Gparticle>";
	}


	std::shared_ptr<GLogger>& log;

public:
	void shootParticle(G4ParticleGun* particleGun, G4Event* anEvent, const std::shared_ptr<GLogger>& log);

private:
	// if this is used somewhere else, it should be moved to gutilities (it does require geant4)
	[[nodiscard]] double randomizeNumberFromSigmaWithModel(double center, double delta, gutilities::randomModel model) const;

	double calculateMomentum();

	double calculateKinEnergy(double mass);

	G4ThreeVector calculateBeamDirection();

	G4ThreeVector calculateVertex();

	// utility methods
	int get_pdg_id(const std::shared_ptr<GLogger>& log);
};
