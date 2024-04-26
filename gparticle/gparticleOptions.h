#ifndef  GPARTICLE_H_OPTIONS
#define  GPARTICLE_H_OPTIONS 1

// glibrary
#include "goptions.h" 

namespace gparticle {

// returns array of options definitions
vector<GOption> defineOptions();


// single particle is a structured option
// ------

struct JParticle {
	string pname;
	int multiplicity;
	
	float p;
	float theta;
	float phi;
	
	float delta_p;
	float delta_theta;
	float delta_phi;
	string thetaModel;
	
	// gaussian spread around values or uniform in range
	string randomMomentumModel;
	
	string punit;
	string aunit;
	
	float vx;
	float vy;
	float vz;
	
	// mutually exclusive vertex distributions
	//
	// xyz components separately
	float delta_vx;
	float delta_vy;
	float delta_vz;
	
	// sphere centered on v. This overwrites
	float delta_VR;
	
	// gaussian spread around values or uniform in range
	string randomVertexModel;
	
	string vunit;
	
};

void from_json(const json& j, JParticle& jpar);

// method to return a vector of JSystem from a structured option
vector<JParticle> getJParticles(GOptions *gopts);

}




#endif
