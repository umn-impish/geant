#include <iostream>

#include <actions.hh>
#include <configs.hh>
#include <construction.hh>

#include <FTFP_BERT.hh>
#include <G4OpticalParameters.hh>
#include <G4OpticalPhysics.hh>
#include <G4PhysListFactory.hh>
#include <G4RadioactiveDecayPhysics.hh>
#include <G4RunManagerFactory.hh>
#include <G4UIExecutive.hh>
#include <G4UImanager.hh>
#include <G4VisExecutive.hh>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

int main(int argc, char *argv[]) {
  int minimumArgs = 3;
  if (argc < minimumArgs) {
    std::cerr << "Usage: " << argv[0]
              << " [simulation config file path] [metadata path] [optional "
                 "macro to run]"
              << std::endl;
    return 1;
  }

  std::random_device rd{};
  std::cout << "random entropy is " << rd.entropy() << std::endl;

  // Generate a random seed and use it
  auto seed = rd();
  std::cout << "seed is " << seed << std::endl;
  G4Random::setTheSeed(seed);

  std::string configFile{argv[1]};

  // Set the file name for this run
  const auto &configInst = GlobalConfigs::instance(configFile);

  auto *runManager =
      G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);

  std::cout << "num cores: " << G4Threading::G4GetNumberOfCores() << std::endl;
  // If the user specifies the number of cores to use,
  // tell the RunManager to use that many
  auto numCores = std::getenv("GEANT4_NUM_CORES");
  runManager->SetNumberOfThreads((numCores == nullptr)
                                     ? G4Threading::G4GetNumberOfCores()
                                     : std::atoi(numCores));

  G4PhysListFactory plf;
  auto *pl = plf.GetReferencePhysList("FTFP_BERT_EMZ");
  pl->RegisterPhysics(new G4OpticalPhysics);
  pl->RegisterPhysics(new G4RadioactiveDecayPhysics);
  runManager->SetUserInitialization(pl);

  // Construction needs the material metadata from JSON file
  runManager->SetUserInitialization(new DetectorConstruction{argv[2]});
  runManager->SetUserInitialization(new ActionInitialization);

  // optionally enable scintillation
  G4OpticalParameters::Instance()->SetProcessActivation(
      "Scintillation", configInst.configOption<bool>("enable-scintillation"));

  // I have found Cherenkov radiation to be error-prone; disable it
  G4OpticalParameters::Instance()->SetProcessActivation("Cerenkov", false);

  auto *uiMan = G4UImanager::GetUIpointer();
  bool batchMode = (argc > minimumArgs);
  if (batchMode) {
    G4String command = "/control/execute ";
    G4String fileName = argv[3];
    uiMan->ApplyCommand("/run/initialize");
    uiMan->ApplyCommand(command + fileName);
  } else {
    auto *ui = new G4UIExecutive{argc, argv};
    G4VisManager *visManager = new G4VisExecutive;
    visManager->Initialize();
    uiMan->ApplyCommand("/control/execute macros/init_vis.mac");
    ui->SessionStart();

    delete visManager;
    delete ui;
  }

  delete runManager;

  return 0;
}
