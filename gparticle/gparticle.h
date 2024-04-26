#ifndef  GPARTICLE_H
#define  GPARTICLE_H  1

// gparticle
#include "gparticleOptions.h"

// geant4
#include "G4ThreeVector.hh"
#include "G4ParticleGun.hh"

// c++
using std::ostream;

// This class interfaces to the Geant4 General Particle Source
class Gparticle {
public:

    Gparticle(gparticle::JParticle jparticle);

private:

    // PDG Monte Carlo Particle Numbering Scheme:
    // https://pdg.lbl.gov/2019/reviews/rpp2019-rev-monte-carlo-numbering.pdf
    string name;
    int pid;
    int multiplicity;

    // assigned momentum and distributions
    float p;
    float delta_p;
    float theta;

    float phi;
    float delta_theta;
    float delta_phi;
    string thetaModel;  // 'ct' (default): cosTheta is uniform. 'flat': theta is uniform
    bool momentumGaussianSpread;  // true: gaussian around values, using the deltas as sigmas. false: uniform distribution in speficied range

    // assigned vertex and distributions
    G4ThreeVector v;
    G4ThreeVector delta_v;
    float delta_VR;
    bool vertexGaussianSpread;     // true: gaussian around values. false: uniform distribution in specified range

    int verbosity;
    friend ostream &operator<<(ostream &stream, Gparticle); // Logs infos on screen.

public:

    void shootParticle(G4ParticleGun *particleGun, G4Event *anEvent, int verbosity = 0);


    // methods that uniformly distribute the particles
private:

    float randomize(float center, float delta, bool gaussianSPread);

    float calculateMomentum();

    float calculateKinEnergy(float mass);

    G4ThreeVector calculateBeamDirection();

    G4ThreeVector calculateVertex();

    void set_pdg_id();
    double get_mass();

};


#endif
