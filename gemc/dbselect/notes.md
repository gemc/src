**Geometry reload lifecycle** (GUI setup-tab or programmatic reload):
1. `gdetector->reload_geometry(SystemList)` — builds preview geometry (calls `Construct()` and
   `ConstructSDandField()` on the master thread).
2. `gdetector->prepare_geometry_for_run()` — calls `ReinitializeGeometry` and `Initialize()`; it must run
   before `BeamOn`.
3. `runManager->BeamOn(n)`

**`digiplugins_need_reload` flag** — set by `reload_geometry()` and `prepare_geometry_for_run()`; cleared
after `Construct()` loads the plugins and assigns master-owned logical-volume user limits. Worker
`ConstructSDandField()` calls only bind the completed digitization map to their thread-local sensitive
detectors.

**`GVisManagerGuard`** — anonymous-namespace `G4VVisManager` subclass used to null the concrete vis manager
pointer during `G4*Store::Clean()`, preventing ToolsSG crashes on dangling geometry references.
