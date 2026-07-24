# Geant4 vis: inspect a single volume in a new window

The correct command sequence to open a named logical volume in a fresh viewer without showing the full scene,
with white background and 2D label, then restoring GUI focus to the original viewer:

```cpp
G4UImanager* uim = G4UImanager::GetUIpointer();

// 1. Detect the active driver and save the current viewer name
auto* vm = G4VisManager::GetInstance();
std::string driverName = "TOOLSSG_QT_GLES";   // fallback
std::string originalViewerName;
if (vm) {
    const auto* viewer = vm->GetCurrentViewer();
    if (viewer) {
        originalViewerName = viewer->GetName();
        if (const auto* gs = viewer->GetSceneHandler()->GetGraphicsSystem())
            driverName = gs->GetNickname();
    }
}

// 2. Open new viewer, create + attach a fresh empty scene
uim->ApplyCommand("/vis/open " + driverName);
uim->ApplyCommand("/vis/scene/create");
uim->ApplyCommand("/vis/sceneHandler/attach");   // CRITICAL — wires handler to new scene

// 3. Add the volume (use full logical-volume name, depth -1 = all daughters)
uim->ApplyCommand("/vis/scene/add/volume " + logicalVolumeName + " -1");

// 4. Cosmetics
uim->ApplyCommand("/vis/viewer/set/background 1 1 1 1");
uim->ApplyCommand("/vis/viewer/set/lineSegmentsPerCircle 100");
uim->ApplyCommand("/vis/set/textColour black");
uim->ApplyCommand("/vis/set/textLayout centre");
uim->ApplyCommand("/vis/scene/add/text2D 0 0.85 36 ! ! " + labelText);
uim->ApplyCommand("/vis/viewer/flush");

// 5. Restore GUI focus so subsequent actions apply to the original viewer
if (!originalViewerName.empty())
    uim->ApplyCommand("/vis/viewer/select " + originalViewerName);
```

**Why `/vis/sceneHandler/attach` is required**: `/vis/open` creates a new viewer whose scene handler still
points at the old (full) scene. Without the attach step, the first flush renders the entire detector.

**Volume naming**: GEMC logical volumes follow the `system/volume` convention (e.g. `cherenkov/tank`).
Use the full name in all `/vis/scene/add/volume` commands.

**Default interactive driver**: `GDEFAULTVIEWERDRIVER` (in `g4display/g4displayConventions.h`).
