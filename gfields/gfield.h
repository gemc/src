#ifndef  GFIELDS_H
#define  GFIELDS_H  1

// G4 headers
#include "G4MagneticField.hh"
#include "G4FieldManager.hh"
#include "G4MagIntegratorStepper.hh"

// gemc
#include "gfactory.h"
#include "goptions.h"
#include "gfieldConventions.h"

class GField : public G4MagneticField {

public:
    virtual ~GField() = default;

    virtual void GetFieldValue(const double x[3], double *bfield) const = 0; ///< Pure virtual: must implement GetFieldValue method


    // create the G4FieldManager
    G4FieldManager *create_FieldManager(string int_stepper, double min_step);

private:

    string name; // key in the gmagneto maps
    string integration_stepper;
    double minimum_step;
    int verbosity;

    // logging
    void gFLogMessage(std::string message) {
        gLogMessage(GFIELDLOGHEADER + name + " " + message);
    }

    // hardcoded list, how to make it dynamic?
    vector <string> SUPPORTED_STEPPERS = {
            "G4DormandPrince745",
            "G4ClassicalRK4",
            "G4SimpleRunge",
            "G4HelixExplicitEuler",
            "G4HelixImplicitEuler",
            "G4CashKarpRKF45",
            "G4RKG3_Stepper",
            "G4SimpleHeum",
            "G4NystromRK4",
            "G4ImplicitEuler",
            "G4ExplicitEuler"
    };

// method to dynamically load factories
public:

    static GField *instantiate(const dlhandle handle) {

        if (handle == nullptr) return nullptr;

        // must match the extern C declaration in the derived factories
        void *maker = dlsym(handle, "GFieldFactory");

        if (maker == nullptr) return nullptr;

        typedef GField *(*fptr)();

        // static_cast not allowed here
        // see http://stackoverflow.com/questions/573294/when-to-use-reinterpret-cast
        // need to run the DLL GMediaFactory function that returns the factory
        fptr func = reinterpret_cast<fptr>(reinterpret_cast<void *>(maker));

        return func();
    }
};


#endif
