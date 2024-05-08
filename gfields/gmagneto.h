#ifndef  GMAGNETO_H
#define  GMAGNETO_H 1

// gemc
#include "gfield.h"
#include "gfieldOptions.h"
#include "gStateMessage.h"

// a world is a collection of GSystem, their
class GMagneto : public GStateMessage {
public:
    // constructor from a jcard / command lines:
    //
    // - load fields jcard definitions
    // - load fields from $GEMC/shared/fields directory

    GMagneto(GOptions *gopts);

    ~GMagneto() {
        delete gFieldMap;
        delete gFieldMgrMap;
    }

private:

    map<string, GField *> *gFieldMap;
    map<string, G4FieldManager *> *gFieldMgrMap;

public:

    // map<string, GField*>* getFieldsMap() const {return gFieldMap;}
    // map<string, G4FieldManager*>* getFieldMgrMap() const {return gFieldMgrMap;}

//    GField *getField(string name) {
//        if (gFieldMap->find(name) == gFieldMap->end()) {
//
//            // error, exit
//            logError("Field " + name + " not found.", EC__WRONG_FIELD_NOT_FOUND);
//        }
//        return gFieldMap->at(name);
//    }

    G4FieldManager *getFieldMgr(string name) {
        if (gFieldMgrMap->find(name) == gFieldMgrMap->end()) {

            // error, exit
            logError("Field " + name + " not found.", EC__WRONG_FIELD_NOT_FOUND);
        }

        return gFieldMgrMap->at(name);
    }

};


#endif
