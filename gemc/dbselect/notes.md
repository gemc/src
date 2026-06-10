**Geometry reload lifecycle** (GUI setup-tab or programmatic reload):
1. `gdetector->reload_geometry(SystemList)` — builds preview geometry (calls `Construct()` + `ConstructSDandField()` on master thread)
2. `gdetector->prepare_geometry_for_run()` — calls `ReinitializeGeometry` + `Initialize()`; must be called before `BeamOn`
3. `runManager->BeamOn(n)`

**`digiplugins_need_reload` flag** — set by `reload_geometry()` and `prepare_geometry_for_run()`; cleared after `loadDigitizationPlugins()`. Prevents clearing the shared digi map during routine BeamOn re-inits when workers may be reading it.

**`GVisManagerGuard`** — anonymous-namespace `G4VVisManager` subclass used to null the concrete vis manager pointer during `G4*Store::Clean()`, preventing ToolsSG crashes on dangling geometry references.

