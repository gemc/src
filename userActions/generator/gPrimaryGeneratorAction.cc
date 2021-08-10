#include "gPrimaryGeneratorAction.h"


// TODO: to be removed when using joptions?
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"

GPrimaryGeneratorAction::GPrimaryGeneratorAction() : G4VUserPrimaryGeneratorAction()
{
	G4int n_particle = 1;
	fParticleGun = new G4ParticleGun(n_particle);

}

GPrimaryGeneratorAction::~GPrimaryGeneratorAction()
{
	delete fParticleGun;
}


void GPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{

//	G4ThreeVector position = fParticleGun->GetParticlePosition();
//	fParticleGun->SetParticlePosition(position);

	G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
	G4ParticleDefinition* particle = particleTable->FindParticle("proton");
	fParticleGun->SetParticleDefinition(particle);
	fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0.0, 0.05, 0.8));
	fParticleGun->SetParticleEnergy(1.0*GeV);

	G4double position = 10.*cm;
	//
	fParticleGun->SetParticlePosition(G4ThreeVector(0.*cm, 0.*cm, position));



	fParticleGun->GeneratePrimaryVertex(anEvent);
}
