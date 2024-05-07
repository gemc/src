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

private:

    // instantiate stepper based on integration method
    G4MagIntegratorStepper *instantiate_stepper_method(string integration_stepper);

    // Creates the G4 Magnetic Field Manager
    //void create_FieldManager(const GOptions *gopts);

    // logging
    void gFLogMessage(std::string message) {
        gLogMessage(GFIELDLOGHEADER + message);
    }

    // hardcoded list
    vector<string> SUPPORTED_STEPPERS = {
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

    // Returns Magnetic Field Manager Pointer
    // creates one if it doesn't exist
//    G4FieldManager *get_GField_Manager(const GOptions *gopts) {
//        if (fFieldManager == nullptr) {
//            create_FieldManager(gopts);
//        }
//        return fFieldManager;
//    }

    static GField *instantiate(const dlhandle handle) {

        if (handle == nullptr) return nullptr;

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
