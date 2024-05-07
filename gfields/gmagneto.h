#ifndef  GMAGNETO_H
#define  GMAGNETO_H 1

// gfield
#include "gfield.h"
#include "gfieldOptions.h"

// a world is a collection of GSystem, their
class GMagneto
{
public:
    // constructor from a jcard / command lines:
    //
    // - load fields jcard definitions
    // - load fields from $GEMC/shared/fields directory

    GMagneto(GOptions* gopts);
    ~GMagneto() {
        delete gFieldMap;
        delete gFieldMgrMap;
    }

private:

    map<string, GField*> *gFieldMap;
    map<string, G4FieldManager*> *gFieldMgrMap;

public:

    map<string, GField*>* getFieldsMap() const {return gFieldMap;}
    map<string, G4FieldManager*>* getFieldMgrMap() const {return gFieldMgrMap;}

};


#endif
