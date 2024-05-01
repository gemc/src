#ifndef  GFIELDS_H
#define  GFIELDS_H  1

// G4 headers
#include "G4MagneticField.hh"
#include "G4FieldManager.hh"

// gemc
#include "gfactory.h"
#include "goptions.h"
#include "gfieldConventions.h"

class GField : public G4MagneticField {

public:
    virtual ~GField() = default;
    virtual void GetFieldValue(const double x[3], double *bfield) const = 0; ///< Pure virtual: must implement GetFieldValue method
    virtual void create_GField_Manager(const GOptions *gopts) = 0;           ///< Pure virtual: must implement creation of the G4 Magnetic Field Manager

protected:
    G4FieldManager *GField_Manager;  ///< Geant4 Field Manager

// method to dynamically load factories
public:

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

    // Returns Magnetic Field Manager Pointer
    // creates one if it doesn't exist
    G4FieldManager *get_GField_Manager(const GOptions *gopts) {
        if (GField_Manager == nullptr) {
            create_GField_Manager(gopts);
        }
        return GField_Manager;
    }

private:

    // logging
    void gFLogMessage(std::string message) {
        gLogMessage(GFIELDLOGHEADER + message);
    }
};


#endif
