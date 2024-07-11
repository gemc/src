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
                     int amultiplicity,
                     float ap,
                     float adelta_p,
                     string punit,
                     string arandomMomentumModel,
                     float atheta,
                     float adelta_theta,
                     string athetaModel,
                     float aphi,
                     float adelta_phi,
                     string aunit,
                     float avx,
                     float avy,
                     float avz,
                     float adelta_vx,
                     float adelta_vy,
                     float adelta_vz,
                     string vunit,
                     string arandomVertexModel,
                     int averbosity) :
        name(aname),
        multiplicity(amultiplicity),
        p(getG4Number(to_string(ap) + punit)),
        delta_p(getG4Number(to_string(adelta_p) + punit)),
        randomMomentumModel(arandomMomentumModel),
        theta(getG4Number(to_string(atheta) + aunit)),
        delta_theta(getG4Number(to_string(adelta_theta) + aunit)),
        thetaModel(athetaModel),
        phi(getG4Number(to_string(aphi) + aunit)),
        delta_phi(getG4Number(to_string(adelta_phi) + aunit)),
        v(G4ThreeVector(
                getG4Number(to_string(avx) + vunit),
                getG4Number(to_string(avy) + vunit),
                getG4Number(to_string(avz) + vunit)
        )),
        delta_v(G4ThreeVector(
                getG4Number(to_string(adelta_vx) + vunit),
                getG4Number(to_string(adelta_vy) + vunit),
                getG4Number(to_string(adelta_vz) + vunit)
        )),
        randomVertexModel(arandomVertexModel),
        verbosity(averbosity) {

    set_pdg_id();

    // print particle with ostream operator



//    if (verbosity >= GVERBOSITY_DETAILS) {
//        cout << "Gparticle: " << endl;
//        cout << "  name: " << name << " (pid: " << pid << ")" << endl;
//        cout << "  p: " << p << endl;
//        cout << "  delta_p: " << delta_p << endl;
//        cout << "  randomMomentumModel: " << randomMomentumModel << endl;
//        cout << "  thetaModel: " << thetaModel << endl;
//
//        cout << "  theta: " << theta / CLHEP::rad << endl;
//        cout << "  delta_theta: " << delta_theta << endl;
//        cout << "  phi: " << phi << endl;
//        cout << "  delta_phi: " << delta_phi << endl;
//        cout << "  vertexGaussianSpread: " << vertexGaussianSpread << endl;
//        cout << "  v: " << v << endl;
//        cout << "  delta_v: " << delta_v << endl;
//        cout << "  delta_VR: " << delta_VR << endl;
//    }
}


// for G4GeneralParticleSource API check:
// https://geant4.kek.jp/lxr/source/event/include/G4ParticleGun.hh
// https://geant4.kek.jp/lxr/source/event/src/G4ParticleGun.cc
void Gparticle::shootParticle(G4ParticleGun *particleGun, G4Event *anEvent, [[maybe_unused]] int verb) {

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

        } else {
            cerr << FATALERRORL << " particle >" << name << "< not found in G4ParticleTable* " << particleTable << endl;
            gexit(EC__GPARTICLENOTFOUND);
        }

    } else {
        cerr << FATALERRORL << "G4ParticleTable not found - G4ParticleGun*: " << particleGun << endl;
        gexit(EC__GPARTICLETABLENOTFOUND);
    }

}


float Gparticle::calculateMomentum() {

    float pmev = randomize(p / CLHEP::MeV, delta_p / CLHEP::MeV, momentumGaussianSpread);

    return pmev;
}

float Gparticle::calculateKinEnergy(float mass) {

    float pmev = calculateMomentum();

    return sqrt(pmev * pmev + mass * mass) - mass;

}


G4ThreeVector Gparticle::calculateBeamDirection() {

    double thetaRad = 0;

    if (thetaModel == "ct") {

        double lower = (theta - delta_theta) / CLHEP::rad;
        double upper = (theta + delta_theta) / CLHEP::rad;

        if (lower < upper) {
            // generate random cos(theta) in range [lower, upper]
            do {
                thetaRad = acos(1 - 2 * G4UniformRand());
            } while (thetaRad < lower || thetaRad > upper);
        } else {
            thetaRad = theta / CLHEP::rad;
        }

        // notice the formula below doesn't work because cos(theta) = cos(-theta)
        // would need to add cases for theta - delta_theta < 0 and theta + delta_theta > pi
        //		thetaRad = acos(G4UniformRand()*(cos(theta - delta_theta/CLHEP::rad) - cos(theta/CLHEP::rad + delta_theta/CLHEP::rad))
        //							 + cos(theta/CLHEP::rad + delta_theta/CLHEP::rad)) / CLHEP::rad;

    } else if (thetaModel == "flat") {
        thetaRad = randomize(theta / CLHEP::rad, delta_theta / CLHEP::rad, momentumGaussianSpread);
    } else {
        cerr << FATALERRORL << " thetaModel >" << thetaModel << "< not recognized." << endl;
        gexit(EC__GPARTICLEWRONGTHETAMODEL);
    }


    double phiRad = randomize(phi / CLHEP::rad, delta_phi / CLHEP::rad, momentumGaussianSpread);

    G4ThreeVector pdir = G4ThreeVector(
            cos(phiRad) * sin(thetaRad),
            sin(phiRad) * sin(thetaRad),
            cos(thetaRad)
    );

    return pdir;
}

G4ThreeVector Gparticle::calculateVertex() {

    float x, y, z;

    if (delta_VR > 0) {
        float radius;
        do {
            x = randomize(0, delta_VR, vertexGaussianSpread);
            y = randomize(0, delta_VR, vertexGaussianSpread);
            z = randomize(0, delta_VR, vertexGaussianSpread);
            radius = x * x + y * y + z * z;
        } while (radius > delta_VR);

        x = x + v.x();
        y = y + v.y();
        z = z + v.z();

    } else {
        x = randomize(v.x(), delta_v.x(), vertexGaussianSpread);
        y = randomize(v.y(), delta_v.y(), vertexGaussianSpread);
        z = randomize(v.z(), delta_v.z(), vertexGaussianSpread);
    }


    G4ThreeVector vertex = G4ThreeVector(x, y, z);

    return vertex;
}


float Gparticle::randomize(float center, float delta, bool gaussianSPread) {
    if (gaussianSPread) {
        return G4RandGauss::shoot(center, delta);
    } else {
        return center + (2.0 * G4UniformRand() - 1.0) * delta;
    }
}

ostream &operator<<(ostream &stream, Gparticle gparticle) {
    stream << "Gparticle: " << endl;

    cout << "  name: " << gparticle.name << " (pid: " << gparticle.pid << ")" << endl;
    cout << "  mass: " << gparticle.get_mass() << endl;
    cout << "  multiplicity: " << gparticle.multiplicity << endl;
    cout << "  thetaModel: " << gparticle.thetaModel << endl;
    cout << "  p: " << gparticle.p << endl;
    cout << "  delta_p: " << gparticle.delta_p << endl;
    cout << "  theta: " << gparticle.theta / CLHEP::rad << endl;
    cout << "  delta_theta: " << gparticle.delta_theta << endl;
    cout << "  phi: " << gparticle.phi << endl;
    cout << "  delta_phi: " << gparticle.delta_phi << endl;
    cout << "  momentumGaussianSpread: " << gparticle.momentumGaussianSpread << endl;
    cout << "  vertexGaussianSpread: " << gparticle.vertexGaussianSpread << endl;
    cout << "  v: " << gparticle.v << endl;
    cout << "  delta_v: " << gparticle.delta_v << endl;
    cout << "  delta_VR: " << gparticle.delta_VR << endl;

    return stream;
}


void Gparticle::set_pdg_id() {
    auto particleTable = G4ParticleTable::GetParticleTable();

    if (particleTable) {
        auto particleDef = particleTable->FindParticle(name);

        if (particleDef) {
            pid = particleDef->GetPDGEncoding();
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
