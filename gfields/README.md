# gfields

The Geant4 examples usually define a `FieldSetup` class containing:

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

However, it is not necessary to keep all these variables in the class. The custom class can be
derived from `G4FieldManager`, with:

- `G4Mag_UsualEqRhs`, `G4MagIntegratorStepper`, `G4ChordFinder`, and `G4FieldManager` defined in `create_FieldManager`
- `GetFieldValue` as the pure virtual method implemented by the various factories
