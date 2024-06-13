#include <GlobalConfigs.hh>
#include "DetectorConstruction.hh"
#include "ActionInitialization.hh"
#include "PhysicsList.hh"
#include "FTFP_BERT.hh"
#include "QGSP_BIC.hh"
#include <G4PhysListFactory.hh>

#include "G4RunManagerFactory.hh"

#include "G4UImanager.hh"

#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"

#include "Randomize.hh"
#include "G4OpticalParameters.hh"

int main(int argc, char* argv[])
{
  G4UIExecutive* ui = nullptr;
  if (argc < 2) {
      std::cerr << "must provide at least simulation config file as first argument" << std::endl;
      return -1;
  }

  if (argc == 2) {
    ui = new G4UIExecutive(argc, argv);
  }

  namespace ifg = ImpressForGrips;
  if (argc > 1) {
    ifg::GlobalConfigs::instance(argv[1]);
  }

  auto* runManager =
    G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);

  G4PhysListFactory plf;
  constexpr auto plName = "QGSP_BIC_HP_EMZ";
  auto* pl = plf.GetReferencePhysList(plName);
  pl->RegisterPhysics(new G4OpticalPhysics);
  runManager->SetUserInitialization(pl);
  // auto* physicsList = new FTFP_BERT;
  // physicsList->RegisterPhysics( new G4RadioactiveDecayPhysics );
  // runManager->SetUserInitialization( physicsList );
  runManager->SetUserInitialization(new ifg::DetectorConstruction());
  runManager->SetUserInitialization(new ifg::ActionInitialization());

  const char* envNumCores = std::getenv("SLURM_NUM_CORES");
  size_t numCores;
  if (envNumCores != nullptr) {
      std::stringstream ss(envNumCores);
      ss >> numCores;
  }
  else {
      numCores = G4Threading::G4GetNumberOfCores();
  }
  runManager->SetNumberOfThreads(numCores);
  
  G4VisManager* visManager = new G4VisExecutive;
  visManager->Initialize();

  // need to enable scintillation
  // you can disable it for testing
  G4OpticalParameters::Instance()->SetProcessActivation("Scintillation", true);
  // I have found Cherenkov radiation to be error-prone
  G4OpticalParameters::Instance()->SetProcessActivation("Cerenkov", false);

  auto* uiMan = G4UImanager::GetUIpointer();
  uiMan->ApplyCommand("/run/initialize");
  bool batchMode = (ui == nullptr);
  if (batchMode) { 
    G4String command = "/control/execute ";
    G4String fileName = argv[2];
    uiMan->ApplyCommand(command + fileName);
  }
  else { 
    uiMan->ApplyCommand("/control/execute macros/init-vis.mac");
    ui->SessionStart();
    delete ui;
  }
  
  delete visManager;
  delete runManager;
}
