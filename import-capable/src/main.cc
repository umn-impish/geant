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


json inputCadJson();

int main(int argc, char* argv[]) {
    // assume same file name always (... ?)
    auto root = std::filesystem::canonical("/proc/self/exe").parent_path();
    std::string configFile{root / "simulation.config.file"};

    // Set the file name for this run
    GlobalConfigs::instance(configFile);

    // Get the metadata into JSON from stdin
    // we are assuming that it gets passed via something
    // like `cat`: `cat file | ./basic-optical`
    auto cadFileMetadata = inputCadJson();
    std::vector<ConstructionMeta> met;
    for (auto const& dat : cadFileMetadata) {
        ConstructionMeta m{
            .file = dat["file"],
            .material = dat["material"],
            .type = dat["type"],
            .scale = dat["scale"],
            .translation = dat["translation"],
            .euler_rotation = dat["euler_rotation"]
        };
        met.push_back(m);
    }

    auto* ui = new G4UIExecutive(argc, argv);

    auto* runManager =
        G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);

    G4PhysListFactory plf;
    auto* pl = plf.GetReferencePhysList("FTFP_BERT");
    pl->RegisterPhysics(new G4OpticalPhysics);
    runManager->SetUserInitialization(pl);

    runManager->SetUserInitialization(new DetectorConstruction{std::move(met)});
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

json inputCadJson() {
    std::string total, current;
    while (std::cin >> current) {
        total += current;
    }

    return json::parse(total);
}
