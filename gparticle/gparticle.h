#ifndef  GPARTICLE_H
#define  GPARTICLE_H  1

// geant4
#include "G4ThreeVector.hh"
#include "G4ParticleGun.hh"

// c++
using std::ostream;

// This class interfaces to the Geant4 General Particle Source
class Gparticle {

public:

    // Constructor based on parameters
    Gparticle(string name,
              int multiplicity,
              float p,
              float delta_p,
              string punit,
              string randomMomentumModel,
              float theta,
              float delta_theta,
              string thetaModel,
              float phi,
              float delta_phi,
              string aunit,
              float avx,
              float avy,
              float avz,
              float adelta_vx,
              float adelta_vy,
              float adelta_vz,
              string vunit,
              string randomVertexModel,
              int verbosity);

private:

    // PDG Monte Carlo Particle Numbering Scheme:
    // https://pdg.lbl.gov/ > Reviews Tables and Plots > Mathematical Tools > Numbering Scheme
    string name;
    int pid;
    int multiplicity;

    // assigned momentum and distributions. See particle options for description
    float p;
    float delta_p;
    string randomMomentumModel;

    float theta;
    float delta_theta;
    string thetaModel;

    float phi;
    float delta_phi;

    G4ThreeVector v;
    G4ThreeVector delta_v;
    bool randomVertexModel;

    int verbosity;

    // utility methods
    void set_pdg_id();
    double get_mass();

    friend ostream &operator<<(ostream &stream, Gparticle); // Logs infos on screen.

public:

    void shootParticle(G4ParticleGun *particleGun, G4Event *anEvent, int verbosity = 0);


private:

    float randomize(float center, float delta, bool gaussianSPread);

    float calculateMomentum();

    float calculateKinEnergy(float mass);

    G4ThreeVector calculateBeamDirection();

    G4ThreeVector calculateVertex();



};


#endif
