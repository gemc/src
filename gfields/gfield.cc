// G4 headers
#include "G4Mag_UsualEqRhs.hh"
#include "G4DormandPrince745.hh"
#include "G4ClassicalRK4.hh"
#include "G4HelixSimpleRunge.hh"
#include "G4HelixExplicitEuler.hh"
#include "G4HelixImplicitEuler.hh"
#include "G4CashKarpRKF45.hh"
#include "G4RKG3_Stepper.hh"
#include "G4SimpleHeum.hh"
#include "G4NystromRK4.hh"
#include "G4ImplicitEuler.hh"
#include "G4ExplicitEuler.hh"




// gfield
#include "gfield.h"


G4MagIntegratorStepper *GField::instantiate_stepper_method(std::string integration_stepper) {
    G4Mag_UsualEqRhs *iEquation = new G4Mag_UsualEqRhs(this);

    // make sure integration_stepper is one element of SUPPORTED_STEPPERS
    if (std::find(SUPPORTED_STEPPERS.begin(), SUPPORTED_STEPPERS.end(), integration_stepper) == SUPPORTED_STEPPERS.end()) {
        gFLogMessage("Integration Stepper " + integration_stepper + " not supported. Using default: " + GFIELD_DEFAULT_INTEGRATION_STEPPER);
        integration_stepper = GFIELD_DEFAULT_INTEGRATION_STEPPER;
    }

    if (integration_stepper == "G4DormandPrince745") {
        return new G4DormandPrince745(iEquation);
    } else if (integration_stepper == "G4ClassicalRK4") {
        return new G4ClassicalRK4(iEquation);
    } else if (integration_stepper == "G4SimpleRunge") {
        return new G4HelixSimpleRunge(iEquation);
    } else if (integration_stepper == "G4HelixExplicitEuler") {
        return new G4HelixExplicitEuler(iEquation);
    } else if (integration_stepper == "G4HelixImplicitEuler") {
        return new G4HelixImplicitEuler(iEquation);
    } else if (integration_stepper == "G4CashKarpRKF45") {
        return new G4CashKarpRKF45(iEquation);
    } else if (integration_stepper == "G4RKG3_Stepper") {
        return new G4RKG3_Stepper(iEquation);
    } else if (integration_stepper == "G4SimpleHeum") {
        return new G4SimpleHeum(iEquation);
    } else if (integration_stepper == "G4NystromRK4") {
        return new G4NystromRK4(iEquation);
    } else if (integration_stepper == "G4ImplicitEuler") {
        return new G4ImplicitEuler(iEquation);
    } else if (integration_stepper == "G4ExplicitEuler") {
        return new G4ExplicitEuler(iEquation);
    }

}
