/**
 * @file   g4NativeObjectsFactory.cc
 * @ingroup Geometry
 * @brief  Implementation of G4NativeSystemFactory helpers, including the
 *         parameter‑count validator and @c buildSolid dispatcher.
 */

// g4system
#include "g4NativeObjectsFactory.h"
#include "g4system/g4systemConventions.h"



// ────────────────────────────────────────────────────────────────
// checkAndReturnParameters – validates & parses constructor inputs
// ────────────────────────────────────────────────────────────────

std::vector<double> G4NativeSystemFactory::checkAndReturnParameters(const GVolume* s)
{
    // PRAGMA TODO: check non‑zero pars for various constructor
    // Example: G4Box should not have any dimension equal to zero.
    //
    //  for(unsigned i=0; i<parameters.size(); ++i) {
    //      if(getG4Number(parameters[i]) == 0) {
    //          cout << GWARNING << " Warning: G4Box has one side null!" << endl;
    //      }
    //  }

    std::set<int> possibleNumberOfParameters;
    std::string   type = s->getType();
    std::string   name = s->getName();

    // Convert comma‑/space‑separated string into a vector<double>
    std::vector<double> parameters = gutilities::getG4NumbersFromString(s->getParameters());
    int actualNumberOfParameters   = static_cast<int>(parameters.size());

    // ─── table of valid counts for each primitive ───
    if      (type == "G4Box")              possibleNumberOfParameters = {3};
    else if (type == "G4Tubs")             possibleNumberOfParameters = {5};
    else if (type == "G4CutTubs")          possibleNumberOfParameters = {11};
    else if (type == "G4Cons")             possibleNumberOfParameters = {7};
    else if (type == "G4Para")             possibleNumberOfParameters = {6};
    else if (type == "G4Trd")              possibleNumberOfParameters = {5};
    else if (type == "G4Trap")             possibleNumberOfParameters = {4, 11};
    else if (type == "G4Sphere")           possibleNumberOfParameters = {6};
    else if (type == "G4Orb")              possibleNumberOfParameters = {1};
    else if (type == "G4Torus")            possibleNumberOfParameters = {5};

    // polycones – special rule
    else if (type == "G4Polycone" || type == "G4GenericPolycone")
    {
        // first constructor ➜ (nPars mod 3) == 0
        // second constructor ➜ ((nPars‑3) mod 2) == 0
        if (actualNumberOfParameters % 3 != 0 &&
            (actualNumberOfParameters - 3) % 2 != 0)
        {
            log->error(ERR_G4PARAMETERSMISMATCH,
                        "Wrong number of parameters in the constructor of <",
                        name, "> of solid type <", type, ">: ", s->getParameters());
        }
        return parameters;      // valid – return early
    }

    // polyhedra – special rule
    else if (type == "G4Polyhedra")
    {
        // first constructor ➜ ((nPars‑4) mod 3) == 0
        // second constructor ➜ ((nPars‑4) mod 2) == 0
        if ((actualNumberOfParameters - 4) % 3 != 0 &&
            (actualNumberOfParameters - 4) % 2 != 0)
        {
            log->error(ERR_G4PARAMETERSMISMATCH,
                        "Wrong number of parameters in the constructor of <",
                        name, "> of solid type <", type, ">: ", s->getParameters());
        }
        return parameters;      // valid – return early
    }

    else if (type == "G4EllipticalTube")   possibleNumberOfParameters = {3};
    else if (type == "G4Ellipsoid")        possibleNumberOfParameters = {5};
    else if (type == "G4EllipticalCone")   possibleNumberOfParameters = {4};
    else if (type == "G4Paraboloid")       possibleNumberOfParameters = {3};
    else if (type == "G4Hype")             possibleNumberOfParameters = {5};
    else if (type == "G4Tet")              possibleNumberOfParameters = {12, 13};
    else if (type == "G4TwistedBox")       possibleNumberOfParameters = {4};
    else if (type == "G4TwistedTrap")      possibleNumberOfParameters = {5, 11};
    else if (type == "G4TwistedTrd")       possibleNumberOfParameters = {6};
    else if (type == "G4TwistedTubs")      possibleNumberOfParameters = {5};
    else {
        log->error(ERR_G4SOLIDTYPENOTFOUND,
                    "The constructor of <", name,
                    "> uses an unknown solid type <", type, ">");
    }

    // ─── generic count check ───
    if (possibleNumberOfParameters.find(actualNumberOfParameters)
        == possibleNumberOfParameters.end())
    {
        log->error(ERR_G4PARAMETERSMISMATCH,
                    "Wrong number of parameters in the constructor of <", name,
                    "> of solid type <", type, ">: ", s->getParameters());
    }

    return parameters;
}
