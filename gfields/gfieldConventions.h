#ifndef  GFIELDS_H_CONVENTIONS
#define  GFIELDS_H_CONVENTIONS 1


#define GFIELD_DEFAULT_INTEGRATION_STEPPER "G4DormandPrince745"
#define GFIELD_DEFAULT_MINIMUM_STEP "1.0*mm"

// other default parameters are in the form of strings

// multipoles.
#define GFIELD_DEFAULT_POLE_NUMBER "1"
#define GFIELD_DEFAULT_VERTEX "0*mm"
#define GFIELD_DEFAULT_ROTANGLE "0*deg"
#define GFIELD_DEFAULT_ROTAXIS "Z"
#define GFIELD_DEFAULT_STRENGTH "1.0"
#define GFIELD_DEFAULT_VERBOSITY "0"

// error codes in the 1200
#define EC__WRONG_FIELD_ROTATION 1201
#define EC__WRONG_FIELD_NOT_FOUND 1202
#define EC__STEPPER_NOT_FOUND 1203


#define GFIELDLOGHEADER  "   🧲 GField ▸ "


#endif
