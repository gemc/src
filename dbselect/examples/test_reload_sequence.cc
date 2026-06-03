/**
 * \file test_reload_sequence.cc
 * \brief Exercises reload_geometry → BeamOn → reload_geometry → BeamOn without the GUI.
 *
 * Three-phase sequence:
 *   Phase 1: simple_flux  → BeamOn(10) — expects flux hits
 *   Phase 2: b1           → BeamOn(10) — exercises dosimeter SD setup / teardown
 *   Phase 3: simple_flux  → BeamOn(10) — expects flux hits again (regression check)
 *
 * Primary success criterion: no segfault at any phase boundary.
 * Secondary criterion: flux hits > 0 in phases 1 and 3.
 *
 * Covers the DET1010 / uninitialized-gHitsCollection bug that caused segfaults when
 * switching geometry between runs (stale SD objects reuse / deactivation path).
 */

// dbselect
#include "dbselect_options.h"

// gdetector
#include "gdetectorConstruction.h"

// gsd  (for GHitsCollection typedef)
#include "gsd.h"

// gemc
#include "gsystemConventions.h"

// geant4
#include "G4RunManagerFactory.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4VUserActionInitialization.hh"
#include "G4UserEventAction.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4HCofThisEvent.hh"
#include "G4Event.hh"
#include "G4SystemOfUnits.hh"
#include "QBBC.hh"

// c++
#include <atomic>
#include <iostream>
#include <memory>
#include <string>

// Per-phase hit counters — incremented once per event that contains ≥1 hit.
static std::atomic<int> g_hitsFlux{0};
static std::atomic<int> g_hitsDosimeter{0};

// ---------------------------------------------------------------------------
// Particle gun: proton along +Z from z=-100 mm at 1 GeV.
// Intercepts the simple_flux plane (z=50 mm, 240×240 mm; world half=150 mm).
// ---------------------------------------------------------------------------
class PrimaryGenerator : public G4VUserPrimaryGeneratorAction {
    G4ParticleGun gun;
public:
    PrimaryGenerator() : gun(1) {
        auto* proton = G4ParticleTable::GetParticleTable()->FindParticle("proton");
        gun.SetParticleDefinition(proton);
        gun.SetParticlePosition(G4ThreeVector(0, 0, -100 * mm));
        gun.SetParticleMomentumDirection(G4ThreeVector(0, 0, 1));
        gun.SetParticleEnergy(1 * GeV);
    }
    void GeneratePrimaries(G4Event* ev) override { gun.GeneratePrimaryVertex(ev); }
};

// ---------------------------------------------------------------------------
// Event action: count events that have hits in each SD.
// ---------------------------------------------------------------------------
class EventAction : public G4UserEventAction {
public:
    void EndOfEventAction(const G4Event* event) override {
        // G4HCofThisEvent::GetHC / GetNumberOfCollections are not marked const in G4 11.4.
        auto* hc = const_cast<G4HCofThisEvent*>(event->GetHCofThisEvent());
        if (!hc) return;
        for (G4int i = 0; i < hc->GetNumberOfCollections(); ++i) {
            auto* col = static_cast<GHitsCollection*>(hc->GetHC(i));
            if (!col || col->GetSize() == 0) continue;
            const std::string sdName = col->GetSDname();
            if (sdName == "flux")
                g_hitsFlux.fetch_add(1, std::memory_order_relaxed);
            else if (sdName == "dosimeter")
                g_hitsDosimeter.fetch_add(1, std::memory_order_relaxed);
        }
    }
};

// ---------------------------------------------------------------------------
// Action initialisation — called by Geant4 per worker thread on Initialize().
// ---------------------------------------------------------------------------
class ActionInit : public G4VUserActionInitialization {
public:
    void Build() const override {
        SetUserAction(new PrimaryGenerator);
        SetUserAction(new EventAction);
    }
};

// ---------------------------------------------------------------------------
// Helpers to build a fresh descriptor for each system.
// ---------------------------------------------------------------------------
static auto makeSystem(const std::shared_ptr<GOptions>& gopts, const std::string& dbhost,
                        const std::string& name) {
    return std::make_shared<GSystem>(gopts, dbhost, name,
                                     GSYSTEMSQLITETFACTORYLABEL, "examples", 1, "default");
}

// ---------------------------------------------------------------------------
// Run one phase: reload geometry, prepare for run, BeamOn, return hit counts.
// ---------------------------------------------------------------------------
static void runPhase(const char* label,
                     GDetectorConstruction* gdetector,
                     G4RunManager* rm,
                     SystemList systems,
                     int& outFlux, int& outDosi) {
    g_hitsFlux     = 0;
    g_hitsDosimeter = 0;

    gdetector->reload_geometry(std::move(systems));
    gdetector->prepare_geometry_for_run();
    rm->BeamOn(10);

    outFlux = g_hitsFlux.load();
    outDosi = g_hitsDosimeter.load();
    std::cout << label << ": flux=" << outFlux << " dosimeter=" << outDosi << "\n";
}

// ---------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    auto gopts = std::make_shared<GOptions>(argc, argv, dbselect::defineOptions());

    const std::string dbhost = gopts->getScalarString("sql");

    auto* runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);
    runManager->SetNumberOfThreads(2);
    runManager->SetUserInitialization(new QBBC);

    auto* gdetector = new GDetectorConstruction(gopts);
    runManager->SetUserInitialization(gdetector);
    runManager->SetUserInitialization(new ActionInit);

    std::cout << "\n=== Reload Sequence Test ===\n";

    int flux1 = 0, dosi1 = 0;
    runPhase("Phase 1 (simple_flux)", gdetector, runManager,
             {makeSystem(gopts, dbhost, "simple_flux")}, flux1, dosi1);

    int flux2 = 0, dosi2 = 0;
    runPhase("Phase 2 (b1)",          gdetector, runManager,
             {makeSystem(gopts, dbhost, "b1")},          flux2, dosi2);

    int flux3 = 0, dosi3 = 0;
    runPhase("Phase 3 (simple_flux)", gdetector, runManager,
             {makeSystem(gopts, dbhost, "simple_flux")}, flux3, dosi3);

    std::cout << "\n=== Results ===\n";

    // Stale-SD check: the flux SD must not fire during the b1-only phase.
    bool stale_flux_in_b1   = (flux2 > 0);
    bool stale_dosi_in_flux = (dosi1 > 0 || dosi3 > 0);

    if (stale_flux_in_b1)
        std::cout << "WARNING: unexpected flux hits in Phase 2 (stale SD).\n";
    if (stale_dosi_in_flux)
        std::cout << "WARNING: unexpected dosimeter hits in flux-only phases (stale SD).\n";

    bool passed = (flux1 > 0) && (flux3 > 0) && !stale_flux_in_b1 && !stale_dosi_in_flux;

    delete runManager;

    if (passed) {
        std::cout << "PASSED\n";
        return EXIT_SUCCESS;
    }
    std::cout << "FAILED"
              << (flux1 == 0 ? " (no flux hits in Phase 1)" : "")
              << (flux3 == 0 ? " (no flux hits in Phase 3)" : "")
              << (stale_flux_in_b1   ? " (stale flux SD in Phase 2)"   : "")
              << (stale_dosi_in_flux ? " (stale dosimeter SD in flux phases)" : "")
              << "\n";
    return EXIT_FAILURE;
}
