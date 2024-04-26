// ghit
#include "ghit.h"

// glibrary
#include "gutsConventions.h"

// notice: if the energy deposited is very low (~50eV)
// the rounding error on the averave calculations could be up to 10^-3

void GHit::calculateInfosForBit(int bit)
{
	// bit 0: always present
	if ( bit == 0 ) {

		if(totalEnergyDeposited == UNINITIALIZEDNUMBERQUANTITY) {
			getTotalEnergyDeposited();
		}

		float  avgx = 0,  avgy = 0,  avgz = 0;
		float avglx = 0, avgly = 0, avglz = 0;
		averageTime= 0;

		auto nsteps = edeps.size();
		for ( size_t s=0; s<nsteps; s++) {
			if ( totalEnergyDeposited > 0 ) {
				averageTime += times[s]*edeps[s]/totalEnergyDeposited;
				avgx  += globalPositions[s].getX()*edeps[s]/totalEnergyDeposited;
				avgy  += globalPositions[s].getY()*edeps[s]/totalEnergyDeposited;
				avgz  += globalPositions[s].getZ()*edeps[s]/totalEnergyDeposited;
				avglx += localPositions[s].getX() *edeps[s]/totalEnergyDeposited;
				avgly += localPositions[s].getY() *edeps[s]/totalEnergyDeposited;
				avglz += localPositions[s].getZ() *edeps[s]/totalEnergyDeposited;
			} else {
				averageTime += times[s]/nsteps;
				avgx  += globalPositions[s].getX()/nsteps;
				avgy  += globalPositions[s].getY()/nsteps;
				avgz  += globalPositions[s].getZ()/nsteps;
				avglx += localPositions[s].getX() /nsteps;
				avgly += localPositions[s].getY() /nsteps;
				avglz += localPositions[s].getZ() /nsteps;
			}
		}
		avgGlobalPosition = G4ThreeVector(avgx,  avgy,  avgz);
		avgLocalPosition  = G4ThreeVector(avglx, avgly, avglz);

		if ( processNames.size() > 0) {
			processName = processNames.front();
		}
		
	} else if ( bit == 1 ) {

	} else if ( bit == 2 ) {

	} else if ( bit == 3 ) {

	} else if ( bit == 4 ) {

	}
}

float GHit::getTotalEnergyDeposited()
{
	if(totalEnergyDeposited == UNINITIALIZEDNUMBERQUANTITY) {
		totalEnergyDeposited = 0;

		for(const auto &ei: edeps) {
			totalEnergyDeposited += ei;
		}
	}
	return totalEnergyDeposited;
}

float GHit::getAverageTime()
{
	if(averageTime == UNINITIALIZEDNUMBERQUANTITY) {

		if(totalEnergyDeposited == UNINITIALIZEDNUMBERQUANTITY) {
			getTotalEnergyDeposited();
		}

		averageTime = 0;
		auto nsteps = edeps.size();
		for ( size_t s=0; s<nsteps; s++) {
			if ( totalEnergyDeposited > 0 ) {
				averageTime += times[s]*edeps[s]/totalEnergyDeposited;
			} else {
				averageTime += times[s]/nsteps;
			}
		}

	}

	return averageTime;
}



G4ThreeVector GHit::getAvgGlobaPosition() {

	if ( avgGlobalPosition.getX() == UNINITIALIZEDNUMBERQUANTITY && avgGlobalPosition.getY() == UNINITIALIZEDNUMBERQUANTITY) {

		if(totalEnergyDeposited == UNINITIALIZEDNUMBERQUANTITY) {
			getTotalEnergyDeposited();
		}

		float  avgx = 0, avgy = 0, avgz = 0;

		auto nsteps = edeps.size();
		for ( size_t s=0; s<nsteps; s++) {
			if ( totalEnergyDeposited > 0 ) {
				avgx  += globalPositions[s].getX()*edeps[s]/totalEnergyDeposited;
				avgy  += globalPositions[s].getY()*edeps[s]/totalEnergyDeposited;
				avgz  += globalPositions[s].getZ()*edeps[s]/totalEnergyDeposited;
			} else {
				averageTime += times[s]/nsteps;
				avgx  += globalPositions[s].getX()/nsteps;
				avgy  += globalPositions[s].getY()/nsteps;
				avgz  += globalPositions[s].getZ()/nsteps;
			}
		}
		avgGlobalPosition = G4ThreeVector(avgx,  avgy,  avgz);
	}

	return avgGlobalPosition;
}

G4ThreeVector GHit::getAvgLocalPosition() {

	if ( avgLocalPosition.getX() == UNINITIALIZEDNUMBERQUANTITY && avgLocalPosition.getY() == UNINITIALIZEDNUMBERQUANTITY) {

		if(totalEnergyDeposited == UNINITIALIZEDNUMBERQUANTITY) {
			getTotalEnergyDeposited();
		}


		float  avgx = 0, avgy = 0, avgz = 0;

		auto nsteps = edeps.size();
		for ( size_t s=0; s<nsteps; s++) {
			if ( totalEnergyDeposited > 0 ) {
				avgx  += localPositions[s].getX()*edeps[s]/totalEnergyDeposited;
				avgy  += localPositions[s].getY()*edeps[s]/totalEnergyDeposited;
				avgz  += localPositions[s].getZ()*edeps[s]/totalEnergyDeposited;
			} else {
				averageTime += times[s]/nsteps;
				avgx  += localPositions[s].getX()/nsteps;
				avgy  += localPositions[s].getY()/nsteps;
				avgz  += localPositions[s].getZ()/nsteps;
			}
		}
		avgLocalPosition = G4ThreeVector(avgx,  avgy,  avgz);

	}
	return avgLocalPosition;
}
