#ifndef  GMAGNETO_H
#define  GMAGNETO_H 1

// gemc
#include "gfield.h"
#include "gfieldOptions.h"
#include "gStateMessage.h"

/**
 * @brief Represents a world containing collections of GFields and G4FieldManagers.
 *
 * GMagneto manages the lifecycle and access to magnetic field objects (`GField`) and their
 * corresponding field managers (`G4FieldManager`).
 */
class GMagneto : public GStateMessage {
public:
    /**
     * @brief Constructs a GMagneto object and loads field definitions from options.
     * @param gopts Pointer to the GOptions object containing configuration options.
     */
    GMagneto(GOptions *gopts);

    /**
    * @brief Destructor to clean up dynamically allocated memory.
    */
    ~GMagneto() {
        delete gFieldMap;
        delete gFieldMgrMap;
    }

private:

    std::map<std::string, GField *> *gFieldMap; ///< Map of field names to GField objects.
    std::map<std::string, G4FieldManager *> *gFieldMgrMap; ///< Map of field names to G4FieldManager objects.

public:

    /**
     * @brief Checks if a field with the given name exists.
     * @param name Name of the field to check.
     * @return True if the field exists, false otherwise.
     */
    bool isField(std::string name) {
        return gFieldMap->find(name) != gFieldMap->end();
    }


    /**
     * @brief Retrieves a GField object by its name.
     * @param name Name of the field to retrieve.
     * @return Pointer to the GField object.
     * @throws Logs an error and exits if the field is not found.
     */
    GField *getField(std::string name) {
        bool not_found = (gFieldMap->find(name) == gFieldMap->end());

        if (not_found) {
            logError("GField " + name + " not found.", EC__WRONG_FIELD_NOT_FOUND);
        }
        return gFieldMap->at(name);
    }

    /**
     * @brief Retrieves a G4FieldManager object by its name.
     * @param name Name of the field manager to retrieve.
     * @return Pointer to the G4FieldManager object.
     * @throws Logs an error and exits if the field manager is not found.
     */
    G4FieldManager *getFieldMgr(std::string name) {
        if (gFieldMgrMap->find(name) == gFieldMgrMap->end()) {
            logError("G4FieldManager " + name + " not found.", EC__WRONG_FIELD_NOT_FOUND);
        }

        return gFieldMgrMap->at(name);
    }

};


#endif
