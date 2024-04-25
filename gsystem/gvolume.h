#ifndef  GVOLUME_H
#define  GVOLUME_H 1

// conventions
#include "gsystemConventions.h"

// glibrary, namespace for the inline functions below
#include "gutilities.h"
using namespace gutilities;

// c++
#include <string>
using std::string;
using std::ostream;
using std::vector;

class GVolume
{
public:
	GVolume(string system, vector<string> pars, string importPath = UNINITIALIZEDSTRINGQUANTITY);
	GVolume(string rootVolumeDefinition); // special constructor for root volume

private:
	string         system; // System of provenience
	string           name; // Name of the volume
	string     motherName; // Mother Volume name
	string    description; // Volume Description, for documentation
	string importFilename; // For imports, filename with path, set with the import factory
	
	// solid parameters
	string        type;    // solid type. This follows the GEANT4 definitions
	string  parameters;    // vector of parameters used in the geant4 solid constructor

	// solid visualization style
	bool        visible;   // visibility of the detector: 0=invisible 1=visible
	int           style;   // Visual style: 0=wireframe 1=solid
	string        color;   // 6(7) digits colors in RRGGBB format. Last optional digit is transparency

	// logical attributes
	string     material;   // Volume Material name.
	string     emfield;    // Magnetic Field. Possible choices: "inherit", "fieldName", "noField"

	// physical attributes
	string          pos;   // Defines the position relative to the mother volume
	string          rot;   // Define the rotation Matrix, defined by rotations along x,y,z axis relative to the mother volume
	string        shift;   // Position modifier
	string         tilt;   // Rotation modifier
	bool          exist;   // Existance modifier

	string  digitization;   // Assigns digitization type and collection ID
	string  gidentity;      // String with identifiers in it. Example: 'sector: 2, layer: 4, wire; 33'

	// special cases
	string       copyOf;   // name of gvolume to copy from
	string    replicaOf;   // name of gvolume to replica from
	string    solidsOpr;   // solids operation
	
	int         pCopyNo;   // should be set to 0 for the first volume of a given type

	// mirrors
	string       mirror;

	// the map key names used in geant4 contains the system name
	// these are assigned by gworld after all voumes are loaded
	string g4name;          // Name of the g4volume
	string g4motherName;    // Name of the g4 Mother volume

    // variation and run number for this gvolume
    string variation;
    int    runno;

	friend ostream &operator<<(ostream &stream, GVolume); // Logs infos on screen.

public:
	inline const string getSystem()       const {return system;}
	
	inline const string getName()         const {return name;}
	inline const string getMotherName()   const {return motherName;}
	inline const string getG4Name()       const {return g4name;}
	inline const string getG4MotherName() const {return g4motherName;}

	inline vector<double> getDetectorDimensions() const {
        if (parameters == UNINITIALIZEDSTRINGQUANTITY ) {
            return { 0 };
        } else { return getG4NumbersFromString(parameters); }
    }

	inline const string getType()       const {return type;}
	inline const string getParameters() const {return parameters;}
	inline const string getMaterial()   const {return material;}
	inline  int    getPCopyNo()    const {return pCopyNo;}
	inline const string getColor()      const {return color;}
	inline bool   isVisible()     const {return visible;}
	inline int    getStyle()      const {return style;}
    inline bool   getExistence()  const {return exist;}

	inline const string getPos()   const {return pos;}
	inline const string getRot()   const {return rot;}
	inline const string getShift() const {return shift;}
	inline const string getTilt()  const {return tilt;}

	inline const string  getDigitization() const {return digitization;}
	inline const string  getGIdentity()    const {return gidentity;}

	// special cases
	inline const string getCopyOf()     const {return copyOf;}
	inline const string getReplicaOf()  const {return replicaOf;}
	inline const string getSolidsOpr()  const {return solidsOpr;}

	inline const string getDescription()  const {return description;}

	// assign modifiers
	void applyShift(string s)      {shift = s;}
	void applyTilt(string t)       {tilt  = t;}
	void modifyExistence(bool e)   {exist = e;}
    void resetMotherName(string m) {motherName = m;}
    void setColor(string c)        {color = c;}
    void setMaterial(string m)     {material = m;}
    void setDigitization(string d) {digitization = d;}
    void setGIdentity(string g)    {gidentity = g;}

	// imported volumes
	string getImportedFile() {return importFilename;}
	
	// assign g4names
	inline void assignG4Names(string g4n, string g4m) {g4name = g4n; g4motherName=g4m;}

};


#endif
