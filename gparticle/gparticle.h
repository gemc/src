#ifndef  GPARTICLE_H
#define  GPARTICLE_H  1

// gemc
#include "glogger.h"

// geant4
#include "G4ThreeVector.hh"
#include "G4ParticleGun.hh"

// c++
#include <string>


// This class interfaces to the Geant4 General Particle Source
class Gparticle {

public:
	// Constructor based on parameters
	Gparticle(std::string name,
	          int         multiplicity,
	          float       p,
	          float       delta_p,
	          std::string punit,
	          std::string randomMomentumModel,
	          float       theta,
	          float       delta_theta,
	          std::string thetaModel,
	          float       phi,
	          float       delta_phi,
	          std::string aunit,
	          float       avx,
	          float       avy,
	          float       avz,
	          float       adelta_vx,
	          float       adelta_vy,
	          float       adelta_vz,
	          std::string vunit,
	          std::string randomVertexModel,
	          std::shared_ptr<GLogger> log) ;

private:
	// PDG Monte Carlo Particle Numbering Scheme:
	// https://pdg.lbl.gov/ > Reviews Tables and Plots > Mathematical Tools > Numbering Scheme
	std::string name;
	int         pid;
	int         multiplicity;

	// assigned momentum and distributions. See particle options for description
	float                   p;
	float                   delta_p;
	gutilities::randomModel randomMomentumModel;

	float                   theta;
	float                   delta_theta;
	gutilities::randomModel randomThetaModel;

	float phi;
	float delta_phi;

	G4ThreeVector           v;
	G4ThreeVector           delta_v;
	gutilities::randomModel randomVertexModel;


	double get_mass();

	friend std::ostream& operator<<(std::ostream& stream, Gparticle); // Logs infos on screen.

public:
	void shootParticle(G4ParticleGun* particleGun, G4Event* anEvent, std::shared_ptr<GLogger> log);

private:
	// if this is used somewhere else it should be moved to gutilities (it does require geant4)
	float randomizeNumberFromSigmaWithModel(float center, float delta, gutilities::randomModel model);

	float calculateMomentum();

	float calculateKinEnergy(float mass);

	G4ThreeVector calculateBeamDirection();

	G4ThreeVector calculateVertex();

	// utility methods
	int get_pdg_id(std::shared_ptr<GLogger> log);
};


#endif
