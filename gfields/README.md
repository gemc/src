# gfields

On the g4 examples they usually define a class "FieldSetup" containing:

```
G4FieldManager*          fFieldManager = nullptr;
G4ChordFinder*           fChordFinder = nullptr;
G4Mag_UsualEqRhs*        fEquation = nullptr;
G4MagneticField*         fMagneticField = nullptr;

G4MagIntegratorStepper*  fStepper = nullptr;
G4bool                   fUseFSALstepper = false;
G4VIntegrationDriver*    fDriver =  nullptr;  // If non-null, its new type (FSAL)
G4int                    fStepperType = -1;
```

However, it's not necessary to have all these variables in the class, and the custom class can be 
derived from G4FieldManager, with:

- G4Mag_UsualEqRhs, G4MagIntegratorStepper, G4ChordFinder and G4FieldManager defined in create_FieldManager
- GetFieldValue as the pure virtual method to be implemented by the various factories
