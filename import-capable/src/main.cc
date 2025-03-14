#include <iostream>

#include <construction.hh>
#include <actions.hh>
#include <configs.hh>

#include <FTFP_BERT.hh>
#include <G4OpticalParameters.hh>
#include <G4OpticalPhysics.hh>
#include <G4PhysListFactory.hh>
#include <G4RadioactiveDecayPhysics.hh>
#include <G4RunManagerFactory.hh>
#include <G4UImanager.hh>
#include <G4VisExecutive.hh>
#include <G4UIExecutive.hh>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "supply metadata fn as first argument--optionally supply a .mac file to run" << std::endl;
        return 1;
    }

    // assume same file name always (... ?)
    auto root = std::filesystem::canonical("/proc/self/exe").parent_path();
    std::string configFile{root / "simulation.config.file"};

    // Set the file name for this run
    const auto& configInst = GlobalConfigs::instance(configFile);

    auto* ui = new G4UIExecutive(argc, argv);

    auto* runManager =
        G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);


    std::cout << "num cores: " << G4Threading::G4GetNumberOfCores() << std::endl;
    // If the user specifies the number of cores to use,
    // tell the RunManager to use that many
    auto numCores = std::getenv("GEANT4_NUM_CORES");
    runManager->SetNumberOfThreads(
        (numCores == nullptr)?
            G4Threading::G4GetNumberOfCores() :
            std::atoi(numCores)
    );

    G4PhysListFactory plf;
    auto* pl = plf.GetReferencePhysList("QGSP_BIC_HP_EMZ");
    pl->RegisterPhysics(new G4OpticalPhysics);
    pl->RegisterPhysics(new G4RadioactiveDecayPhysics);
    runManager->SetUserInitialization(pl);

    // Construction needs the material metadata from JSON file
    runManager->SetUserInitialization(new DetectorConstruction{argv[1]});
    runManager->SetUserInitialization(new ActionInitialization);

    // optionally enable scintillation
    G4OpticalParameters::Instance()->SetProcessActivation(
        "Scintillation",
        configInst.configOption<bool>("enable-scintillation")
    );

    // I have found Cherenkov radiation to be error-prone; disable it
    G4OpticalParameters::Instance()->SetProcessActivation("Cerenkov", false);

    auto* uiMan = G4UImanager::GetUIpointer();
    bool batchMode = (argc > 2);
    if (batchMode) {
        G4String command = "/control/execute ";
        G4String fileName = argv[2];
        uiMan->ApplyCommand("/run/initialize");
        uiMan->ApplyCommand(command + fileName);
    } else {
        G4VisManager* visManager = new G4VisExecutive;
        visManager->Initialize();
        uiMan->ApplyCommand("/control/execute macros/init_vis.mac");
        ui->SessionStart();
        delete visManager;
    }

    delete ui;
    delete runManager;

    return 0;
}
