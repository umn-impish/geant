#include <iostream>

#include <construction.hh>
#include <actions.hh>
#include <configs.hh>

#include <FTFP_BERT.hh>
#include <G4OpticalParameters.hh>
#include <G4OpticalPhysics.hh>
#include <G4PhysListFactory.hh>
#include <G4RunManagerFactory.hh>
#include <G4UImanager.hh>
#include <G4VisExecutive.hh>
#include <G4UIExecutive.hh>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "supply metadata fn as sole argument" << std::endl;
        return 1;
    }
    // assume same file name always (... ?)
    auto root = std::filesystem::canonical("/proc/self/exe").parent_path();
    std::string configFile{root / "simulation.config.file"};

    // Set the file name for this run
    GlobalConfigs::instance(configFile);

    auto* ui = new G4UIExecutive(argc, argv);

    auto* runManager =
        G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);

    G4PhysListFactory plf;
    auto* pl = plf.GetReferencePhysList("FTFP_BERT");
    pl->RegisterPhysics(new G4OpticalPhysics);
    runManager->SetUserInitialization(pl);

    // Construction needs the material metadata from JSON file
    runManager->SetUserInitialization(new DetectorConstruction{argv[1]});
    runManager->SetUserInitialization(new ActionInitialization);

    G4VisManager* visManager = new G4VisExecutive;
    visManager->Initialize();

    // optionally enable scintillation
    G4OpticalParameters::Instance()->SetProcessActivation("Scintillation", false);

    // I have found Cherenkov radiation to be error-prone; disable it
    G4OpticalParameters::Instance()->SetProcessActivation("Cerenkov", false);

    auto* uiMan = G4UImanager::GetUIpointer();
    uiMan->ApplyCommand("/control/execute macros/init_vis.mac");
    ui->SessionStart();

    delete ui;
    delete visManager;
    delete runManager;

    return 0;
}