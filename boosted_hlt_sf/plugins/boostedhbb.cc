// class declaration
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectronFwd.h"
#include "DataFormats/GsfTrackReco/interface/GsfTrack.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonSelectors.h"
#include "DataFormats/VertexReco/interface/Vertex.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/JetReco/interface/GenJetCollection.h"
#include "DataFormats/JetReco/interface/GenJetCollection.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/MET.h"
#include "DataFormats/PatCandidates/interface/PackedGenParticle.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "TLorentzVector.h"
#include "L1Trigger/L1TGlobal/interface/L1TGlobalUtil.h"
#include "CondFormats/DataRecord/interface/L1TUtmTriggerMenuRcd.h"
#include "DataFormats/L1GlobalTrigger/interface/L1GlobalTriggerReadoutRecord.h"

#include "TTree.h"
#include "TH1.h"
#include "fastjet/Selector.hh"
#include "fastjet/PseudoJet.hh"
#include "fastjet/JetDefinition.hh"
#include "fastjet/ClusterSequence.hh"
#include <fastjet/GhostedAreaSpec.hh>
#include "fastjet/GhostedAreaSpec.hh"
#include "fastjet/tools/Filter.hh"
#include "fastjet/tools/Pruner.hh"
#include "fastjet/tools/MassDropTagger.hh"
#include "fastjet/tools/JetMedianBackgroundEstimator.hh"
#include "fastjet/tools/GridMedianBackgroundEstimator.hh"
#include "fastjet/tools/Subtractor.hh"
#include "fastjet/Selector.hh"

#include "DataFormats/PatCandidates/interface/Jet.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/Utilities/interface/InputTag.h"

using namespace fastjet;
using namespace std;
namespace
{
  bool safeAccept(const edm::Handle<edm::TriggerResults> &results,
                  const edm::TriggerNames &names,
                  const std::string &path)
  {
    if (!results.isValid())
      return false;
    const unsigned int idx = names.triggerIndex(path);
    if (idx >= results->size())
      return false;
    return results->accept(idx);
  }

  std::string resolveHLTPath(const edm::TriggerNames &names,
                             const std::string &prefix)
  {
    for (unsigned int i = 0; i < names.size(); ++i)
    {
      const std::string &name = names.triggerName(i);
      if (name.find(prefix) == 0)
        return name;
    }
    return "";
  }

  bool safeFilterAccept(const edm::Handle<edm::TriggerResults> &results,
                        const edm::TriggerNames &names,
                        const std::string &filterName,
                        bool defaultValue = false)
  {
    if (!results.isValid())
      return defaultValue;
    const unsigned int idx = names.triggerIndex(filterName);
    if (idx >= results->size())
      return defaultValue;
    return results->accept(idx);
  }

  template <typename T>
  bool validHandle(const edm::Handle<T> &h)
  {
    return h.isValid();
  }
}
class boostedhbb : public edm::one::EDAnalyzer<edm::one::SharedResources>
{
public:
  explicit boostedhbb(const edm::ParameterSet &);
  ~boostedhbb();

  static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);

private:
  int getNPU(edm::Handle<std::vector<PileupSummaryInfo>> puInfo);
  virtual void analyze(const edm::Event &, const edm::EventSetup &) override;

  edm::EDGetTokenT<edm::View<pat::MET>> metToken_;
  edm::EDGetTokenT<edm::View<pat::Electron>> tok_electrons_;
  edm::EDGetTokenT<pat::MuonCollection> tok_muons_;
  edm::EDGetTokenT<edm::View<pat::Jet>> tok_jet_;
  edm::EDGetTokenT<edm::View<pat::Jet>> tok_pfjetAK8s_;
  edm::EDGetTokenT<std::vector<pat::Jet>> ak15JetsToken_;
  std::string subJetCollectionName;
  edm::EDGetTokenT<std::vector<PileupSummaryInfo>> pileupToken;
  edm::EDGetTokenT<double> rhoToken;
  edm::EDGetTokenT<reco::VertexCollection> tok_primaryVertices_;
  edm::EDGetTokenT<edm::TriggerResults> HLTriggerResults_;
  edm::EDGetTokenT<std::vector<pat::TriggerObjectStandAlone>> triggerObjects_;
  edm::EDGetTokenT<edm::TriggerResults> metfilterspatLabel_;
  edm::EDGetTokenT<edm::TriggerResults> metfiltersrecoLabel_;
  edm::EDGetTokenT<std::vector<pat::MET>> puppimetToken_;
  edm::EDGetTokenT<std::vector<reco::Vertex>> verticesToken_;
  edm::EDGetTokenT<GlobalAlgBlkBxCollection> l1AlgosToken;
  edm::ESGetToken<L1TUtmTriggerMenu, L1TUtmTriggerMenuRcd> l1GtMenuToken_;
  edm::EDGetTokenT<GenEventInfoProduct> genInfoToken_;
  edm::EDGetToken l1GtToken_;
  bool is_MC_;

  TTree *m_tree;
  TTree *m_tree2;
  unsigned long _run, _event, _lumi;
  Float_t _genWeight;
  Int_t nPV;
  Int_t nPU;
  Float_t dxy_cut = 0.05;
  Float_t dz_cut = 0.1;

  Float_t pu = -999.9;
  Float_t rho = -999.9;
  Float_t electron_pt = -999.9, electron_eta = -999.9, electron_phi = -999.9, electron_energy = -999.9;
  Float_t muon_pt = -999.9, muon_eta = -999.9, muon_phi = -999.9, muon_energy = -999.9;
  Int_t electron_charge = 0, muon_charge = 0;

  Int_t njet;
  vector<double> jet_pt;
  vector<double> jet_eta;
  vector<double> jet_phi;
  vector<double> jet_en;
  vector<double> jet_mass;
  vector<double> jet_PFBTG;
  vector<Bool_t> jet_id;
  vector<int> jet_pflv;
  vector<int> jet_hflv;

  Int_t njetak8;
  std::vector<float> jetAK8_pt;
  std::vector<float> jetAK8_pt_raw;
  std::vector<float> jetAK8_eta;
  std::vector<float> jetAK8_phi;
  std::vector<float> jetAK8_mass;
  std::vector<float> jetAK8_area;
  std::vector<float> jetAK8_mass_raw;
  std::vector<float> jetAK8_pnet_probHbb;
  std::vector<float> jetAK8_pnet_probHcc;
  std::vector<float> jetAK8_pnet_probHqq;
  std::vector<float> jetAK8_pnet_probHgg;
  std::vector<float> jetAK8_pnet_probHtt;
  std::vector<float> jetAK8_pnet_probHtm;
  std::vector<float> jetAK8_pnet_probHte;
  std::vector<float> jetAK8_pnet_probQCD2HF;
  std::vector<float> jetAK8_pnet_probQCD1HF;
  std::vector<float> jetAK8_pnet_probQCD0HF;
  std::vector<float> jetAK8_pnet_HbbVsQCD;

  std::vector<float> jetAK8_pnet_mass;
  std::vector<unsigned int> jetAK8_id;
  std::vector<unsigned int> jetAK8_ncand;
  std::vector<unsigned int> jetAK8_hflav;
  std::vector<int> jetAK8_pflav;
  std::vector<unsigned int> jetAK8_nbhad;
  std::vector<unsigned int> jetAK8_nchad;

  std::vector<int> jetAK8_nSubjet;
  std::vector<float> jetAK8_softdrop_pt;
  std::vector<float> jetAK8_softdrop_eta;
  std::vector<float> jetAK8_softdrop_phi;
  std::vector<float> jetAK8_softdrop_mass;
  std::vector<float> jetAK8_softdrop_pt_raw;
  std::vector<float> jetAK8_softdrop_mass_raw;

  std::vector<float> jetAK8_softdrop_subjet_pt;
  std::vector<float> jetAK8_softdrop_subjet_pt_raw;
  std::vector<float> jetAK8_softdrop_subjet_eta;
  std::vector<float> jetAK8_softdrop_subjet_phi;
  std::vector<float> jetAK8_softdrop_subjet_mass;
  std::vector<float> jetAK8_softdrop_subjet_mass_raw;

  std::vector<float> jetAK8_pnet_pfMassDecorrelatedParticleNetprobXbb, jetAK8_pnet_pfMassDecorrelatedParticleNetprobQCDbb, jetAK8_pnet_pfMassDecorrelatedParticleNetprobQCDcc, jetAK8_pnet_pfMassDecorrelatedParticleNetprobQCDb, jetAK8_pnet_pfMassDecorrelatedParticleNetprobQCDc, jetAK8_pnet_pfMassDecorrelatedParticleNetQCDothers, jetAK8_pnet_pfMassDecorrelatedParticleNetXbbvsQCD, jetAK8_pnet_pfMassDecorrelatedParticleNetXccvsQCD, jetAK8_pnet_pfMassDecorrelatedParticleNetXqqvsQCD, jetAK8_pnet_pfDecorrelatedParticleNetTvsQCD, jetAK8_pnet_pfDecorrelatedParticleNetWvsQCD, jetAK8_pnet_pfDecorrelatedParticleNetZvsQCD;

  int njetak15;
  std::vector<float> ak15_pt_;
  std::vector<float> ak15_eta_;
  std::vector<float> ak15_phi_;
  std::vector<float> ak15_mass_;
  std::vector<float> ak15_msoftdrop_;
  std::vector<int> ak15_nSubjet;
  std::vector<float> ak15_probXbb_;
  std::vector<float> ak15_probXqq_;
  std::vector<float> ak15_probXcc_;
  std::vector<float> ak15_probQCD_;
  std::vector<float> ak15_xbbvsqcd_;
  std::vector<float> ak15_pnet_mass_;
  std::vector<float> ak15_radius_;

  std::vector<unsigned int> ak15_ncand;
  std::vector<unsigned int> ak15_hflav;
  std::vector<int> ak15_pflav;
  std::vector<unsigned int> ak15_nbhad;
  std::vector<unsigned int> ak15_nchad;

  std::vector<float> ak15_glopart_probHbb;
  std::vector<float> ak15_glopart_probHcc;
  std::vector<float> ak15_glopart_probHcs;
  std::vector<float> ak15_glopart_probHqq;
  std::vector<float> ak15_glopart_probHthth;
  std::vector<float> ak15_glopart_probTopbWqq;
  std::vector<float> ak15_glopart_probTopbWq;
  std::vector<float> ak15_glopart_probQCD;
  std::vector<float> ak15_glopart_massCorr;
  std::vector<float> ak15_glopart_massCorrGen;

  // HLT objects
  Int_t nCaloAK8jet;
  vector<double> CaloAK8jet_pt;
  vector<double> CaloAK8jet_eta;
  vector<double> CaloAK8jet_phi;
  vector<double> CaloAK8jet_en;

  Int_t nL3AK8jet1;
  vector<double> L3AK8jet1_pt;
  vector<double> L3AK8jet1_eta;
  vector<double> L3AK8jet1_phi;
  vector<double> L3AK8jet1_en;

  Int_t nL3AK8jet2;
  vector<double> L3AK8jet2_pt;
  vector<double> L3AK8jet2_eta;
  vector<double> L3AK8jet2_phi;
  vector<double> L3AK8jet2_en;

  // MET filters
  Bool_t Flag_goodVertices_;
  Bool_t Flag_globalSuperTightHalo2016Filter_;
  Bool_t Flag_EcalDeadCellTriggerPrimitiveFilter_;
  Bool_t Flag_BadPFMuonFilter_;
  Bool_t Flag_BadPFMuonDzFilter_;
  Bool_t Flag_hfNoisyHitsFilter_;
  Bool_t Flag_eeBadScFilter_;
  Bool_t Flag_ecalBadCalibFilter_;

  Float_t HT = -999.9;
  Float_t MET_pt;
  Float_t MET_phi;

  Bool_t HLT_DIJET_BTG = false;
  Bool_t HLT_QUADJ_BTG = false;
  Bool_t isQUAD_JET = false;
  Bool_t HLT_QUADJ_AN1 = false;
  Bool_t HLT_QUADJ_AN2 = false;
  Bool_t HLT_QUADJ_AN3 = false;
  Bool_t HLT_DIJET_CONTROL = false;
  // Bool_t HLT_AK8PFJet230_SoftDropMass40_PNetBB0p10= false, HLT_AK8PFJet230_SoftDropMass40_PNetBB0p10_PNetBB0p06= false, HLT_AK8PFJet230_SoftDropMass40_PNetBB0p10_PNetBB0p10= false;
  // Bool_t HLT_AK8PFJet230_SoftDropMass40_PNetBB0p10 = false;
  Bool_t HLT_QUADJ = false;
  Bool_t HLT_DIJET = false;
  Bool_t HLT_SIGJ = false;

  Bool_t HLT_IsoMu50_AK8PFJet230_SoftDropMass40 = false;
  Bool_t HLT_IsoMu50_AK8PFJet230_SoftDropMass40_PNetBB0p06 = false;
  Bool_t HLT_IsoMu50_AK8PFJet230_SoftDropMass40_PNetBB0p10 = false;
  Bool_t HLT_AK8PFJet230_SoftDropMass40_PNetBB0p06 = false;
  Bool_t HLT_AK8PFJet230_SoftDropMass40_PNetBB0p10 = false;
  Bool_t HLT_AK8PFJet230_SoftDropMass40_PNetTauTau0p03 = false;
  Bool_t HLT_AK8PFJet230_SoftDropMass40_PNetTauTau0p05 = false;
  Bool_t HLT_AK8PFJet230_SoftDropMass40_PNetV02BB0p14 = false;
  Bool_t HLT_AK8PFJet230_SoftDropMass40_PNetV02BB0p26 = false;
  Bool_t HLT_AK15PFJet175_SoftDropMass40 = false;
  Bool_t HLT_AK15PFJet175_SoftDropMass40_GloParTBB0p125 = false;
  Bool_t HLT_AK15PFJet175_SoftDropMass40_GloParTCC0p25 = false;
};
struct JetIDVars
{
  float NHF, NEMF, MUF, CHF, CEMF;
  int NumConst, NumNeutralParticle, CHM;
};
bool getJetID(JetIDVars vars, string jettype = "CHS", string era = "2022FG", double eta = 0)
{
  // https://twiki.cern.ch/twiki/bin/view/CMS/JetID13p6TeV
  if (jettype != "CHS" && jettype != "PUPPI")
  {
    cout << "Don't know your jet type! I know only CHS & PUPPI :D" << endl;
    return false;
  }
  if (era != "2022ABCD" && era != "2022FG")
  {
    cout << "Incorrect era for Run-3, please specify it correctly" << endl;
    return false;
  }
  float NHF, NEMF, MUF, CHF, CEMF;
  int NumConst, NumNeutralParticle, CHM;

  NHF = vars.NHF;
  NEMF = vars.NEMF;
  MUF = vars.MUF;
  CHF = vars.CHF;
  CEMF = vars.CEMF;
  NumConst = vars.NumConst;
  NumNeutralParticle = vars.NumNeutralParticle;
  CHM = vars.CHM;
  bool JetID = false;

  if (jettype == "PUPPI" && era == "2022ABCD")
  {
    JetID = ((abs(eta) <= 2.6 && CEMF < 0.8 && CHM > 0 && CHF > 0.01 && NumConst > 1 && NEMF < 0.9 && MUF < 0.8 && NHF < 0.9) || (abs(eta) > 2.6 && abs(eta) <= 2.7 && CEMF < 0.8 && NEMF < 0.99 && MUF < 0.8 && NHF < 0.9) || (NHF < 0.9999 && abs(eta) > 2.7 && abs(eta) <= 3.0) || (NEMF < 0.90 && NumNeutralParticle > 2 && abs(eta) > 3.0));
  }
  if (jettype == "CHS" && era == "2022ABCD")
  {
    JetID = ((abs(eta) <= 2.6 && CEMF < 0.8 && CHM > 0 && CHF > 0.01 && NumConst > 1 && NEMF < 0.9 && MUF < 0.8 && NHF < 0.9) || (abs(eta) > 2.6 && abs(eta) <= 2.7 && CEMF < 0.8 && CHM > 0 && NEMF < 0.99 && MUF < 0.8 && NHF < 0.9) || (NEMF < 0.99 && NumNeutralParticle > 1 && abs(eta) > 2.7 && abs(eta) <= 3.0) || (NEMF < 0.90 && NumNeutralParticle > 10 && abs(eta) > 3.0));
  }
  if (jettype == "PUPPI" && era == "2022FG")
  {
    JetID = ((abs(eta) <= 2.6 && CEMF < 0.8 && CHM > 0 && CHF > 0.01 && NumConst > 1 && NEMF < 0.9 && MUF < 0.8 && NHF < 0.99) || (abs(eta) > 2.6 && abs(eta) <= 2.7 && CEMF < 0.8 && NEMF < 0.99 && MUF < 0.8 && NHF < 0.9) || (NHF < 0.9999 && abs(eta) > 2.7 && abs(eta) <= 3.0) || (NEMF < 0.90 && NumNeutralParticle >= 2 && abs(eta) > 3.0));
  }
  if (jettype == "CHS" && era == "2022FG")
  {
    JetID = ((abs(eta) <= 2.6 && CEMF < 0.8 && CHM > 0 && CHF > 0.01 && NumConst > 1 && NEMF < 0.9 && MUF < 0.8 && NHF < 0.99) || (abs(eta) > 2.6 && abs(eta) <= 2.7 && CEMF < 0.8 && CHM > 0 && NEMF < 0.99 && MUF < 0.8 && NHF < 0.9) || (NEMF < 0.99 && NumNeutralParticle > 1 && abs(eta) > 2.7 && abs(eta) <= 3.0) || (NEMF < 0.90 && NumNeutralParticle > 10 && abs(eta) > 3.0));
  }
  return JetID;
}

bool Muon_Tight_ID(bool muonisGL, bool muonisPF, float muonchi, float muonhit, float muonmst, float muontrkvtx, float muondz, float muonpixhit, float muontrklay)
{
  // https://twiki.cern.ch/twiki/bin/viewauth/CMS/SWGuideMuonIdRun2#Tight_Muon
  bool tightid = false;
  if (muonisGL && muonisPF)
  {
    if (muonchi < 10 && muonhit > 0 && muonmst > 1)
    {
      if (fabs(muontrkvtx) < 0.2 && fabs(muondz) < 0.5)
      {
        if (muonpixhit > 0 && muontrklay > 5)
        {
          tightid = true;
        }
      }
    }
  }
  return tightid;
}

boostedhbb::boostedhbb(const edm::ParameterSet &iConfig)
{
  // now do what ever initialization is needed
  tok_electrons_ = consumes<edm::View<pat::Electron>>(iConfig.getParameter<edm::InputTag>("Electrons"));
  tok_muons_ = consumes<pat::MuonCollection>(iConfig.getParameter<edm::InputTag>("Muons"));
  tok_jet_ = consumes<edm::View<pat::Jet>>(iConfig.getParameter<edm::InputTag>("Jets"));
  tok_pfjetAK8s_ = consumes<edm::View<pat::Jet>>(iConfig.getParameter<edm::InputTag>("PFJetsAK8"));
  ak15JetsToken_ = consumes<std::vector<pat::Jet>>(iConfig.getParameter<edm::InputTag>("ak15Jets"));
  subJetCollectionName = iConfig.getUntrackedParameter<string>("subjets");
  pileupToken = consumes<std::vector<PileupSummaryInfo>>(iConfig.getUntrackedParameter<edm::InputTag>("pileupInfo"));
  rhoToken = consumes<double>(iConfig.getParameter<edm::InputTag>("labe_rho"));
  HLTriggerResults_ = consumes<edm::TriggerResults>(iConfig.getParameter<edm::InputTag>("bits"));
  triggerObjects_ = consumes<std::vector<pat::TriggerObjectStandAlone>>(iConfig.getParameter<edm::InputTag>("objects"));
  tok_primaryVertices_ = consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("vertices"));
  metfilterspatLabel_ = consumes<edm::TriggerResults>(iConfig.getUntrackedParameter<edm::InputTag>("metfilterspatLabel_"));
  metfiltersrecoLabel_ = consumes<edm::TriggerResults>(iConfig.getUntrackedParameter<edm::InputTag>("metfiltersrecoLabel_"));
  puppimetToken_ = consumes<std::vector<pat::MET>>(iConfig.getParameter<edm::InputTag>("PuppiMet"));
  verticesToken_ = consumes<std::vector<reco::Vertex>>(iConfig.getParameter<edm::InputTag>("Vertices"));
  genInfoToken_ = consumes<GenEventInfoProduct>(iConfig.getParameter<edm::InputTag>("GenInf"));
  l1GtMenuToken_ = esConsumes<L1TUtmTriggerMenu, L1TUtmTriggerMenuRcd>();
  l1GtToken_ = consumes<BXVector<GlobalAlgBlk>>(iConfig.getParameter<edm::InputTag>("l1GtSrc"));
  is_MC_ = iConfig.getParameter<bool>("is_MC");
  usesResource("TFileService");
  edm::Service<TFileService> fs;
  m_tree = fs->make<TTree>("tree", "");
  m_tree2 = fs->make<TTree>("allEvt", "");
  // tree to store all events
  m_tree2->Branch("nPU", &nPU);
  m_tree2->Branch("weight", &_genWeight);

  m_tree->Branch("nPU", &nPU);
  m_tree->Branch("weight", &_genWeight);
  m_tree->Branch("nPV", &nPV);

  m_tree->Branch("rho", &rho);
  m_tree->Branch("run", &_run, "run/l");
  m_tree->Branch("event", &_event, "event/l");
  m_tree->Branch("lumi", &_lumi, "lumi/l");

  m_tree->Branch("Flag_goodVertices", &Flag_goodVertices_);
  m_tree->Branch("Flag_globalSuperTightHalo2016Filter", &Flag_globalSuperTightHalo2016Filter_);
  m_tree->Branch("Flag_EcalDeadCellTriggerPrimitiveFilter", &Flag_EcalDeadCellTriggerPrimitiveFilter_);
  m_tree->Branch("Flag_BadPFMuonFilter", &Flag_BadPFMuonFilter_);
  m_tree->Branch("Flag_BadPFMuonDzFilter", &Flag_BadPFMuonDzFilter_);
  m_tree->Branch("Flag_hfNoisyHitsFilter", &Flag_hfNoisyHitsFilter_);
  m_tree->Branch("Flag_eeBadScFilter", &Flag_eeBadScFilter_);
  m_tree->Branch("Flag_ecalBadCalibFilter_", &Flag_ecalBadCalibFilter_);
  // electron
  m_tree->Branch("electron_pt", &electron_pt);
  m_tree->Branch("electron_eta", &electron_eta);
  m_tree->Branch("electron_phi", &electron_phi);
  m_tree->Branch("electron_energy", &electron_energy);
  m_tree->Branch("electron_charge", &electron_charge);
  // muon
  m_tree->Branch("muon_pt", &muon_pt);
  m_tree->Branch("muon_eta", &muon_eta);
  m_tree->Branch("muon_phi", &muon_phi);
  m_tree->Branch("muon_energy", &muon_energy);
  m_tree->Branch("muon_charge", &muon_charge);
  // now we are storing all the informations about the b-Jets
  m_tree->Branch("njet", &njet);
  m_tree->Branch("jet_pt", &jet_pt);
  m_tree->Branch("jet_eta", &jet_eta);
  m_tree->Branch("jet_phi", &jet_phi);
  m_tree->Branch("jet_en", &jet_en);
  m_tree->Branch("jet_mass", &jet_mass);
  m_tree->Branch("jet_PFBTG", &jet_PFBTG);
  m_tree->Branch("jet_id", &jet_id);
  m_tree->Branch("jet_pflv", &jet_pflv);
  m_tree->Branch("jet_hflv", &jet_hflv);
  m_tree->Branch("HT", &HT);
  // storing the information about the AK8 -jets
  m_tree->Branch("njetak8", &njetak8);
  m_tree->Branch("jetAK8_pt", "std::vector<float>", &jetAK8_pt);
  m_tree->Branch("jetAK8_eta", "std::vector<float>", &jetAK8_eta);
  m_tree->Branch("jetAK8_phi", "std::vector<float>", &jetAK8_phi);
  m_tree->Branch("jetAK8_mass", "std::vector<float>", &jetAK8_mass);
  m_tree->Branch("jetAK8_pt_raw", "std::vector<float>", &jetAK8_pt_raw);
  m_tree->Branch("jetAK8_mass_raw", "std::vector<float>", &jetAK8_mass_raw);
  m_tree->Branch("jetAK8_area", "std::vector<float>", &jetAK8_area);
  m_tree->Branch("jetAK8_pnet_probHbb", "std::vector<float>", &jetAK8_pnet_probHbb);
  m_tree->Branch("jetAK8_pnet_probHcc", "std::vector<float>", &jetAK8_pnet_probHcc);
  m_tree->Branch("jetAK8_pnet_probHqq", "std::vector<float>", &jetAK8_pnet_probHqq);
  m_tree->Branch("jetAK8_pnet_probHgg", "std::vector<float>", &jetAK8_pnet_probHgg);
  m_tree->Branch("jetAK8_pnet_probHtt", "std::vector<float>", &jetAK8_pnet_probHtt);
  m_tree->Branch("jetAK8_pnet_probHtm", "std::vector<float>", &jetAK8_pnet_probHtm);
  m_tree->Branch("jetAK8_pnet_probHte", "std::vector<float>", &jetAK8_pnet_probHte);
  m_tree->Branch("jetAK8_pnet_probQCD2HF", "std::vector<float>", &jetAK8_pnet_probQCD2HF);
  m_tree->Branch("jetAK8_pnet_probQCD1HF", "std::vector<float>", &jetAK8_pnet_probQCD1HF);
  m_tree->Branch("jetAK8_pnet_probQCD0HF", "std::vector<float>", &jetAK8_pnet_probQCD0HF);
  m_tree->Branch("jetAK8_pnet_HbbVsQCD", "std::vector<float>", &jetAK8_pnet_HbbVsQCD);
  m_tree->Branch("jetAK8_pnet_mass", "std::vector<float>", &jetAK8_pnet_mass);
  m_tree->Branch("jetAK8_ncand", "std::vector<unsigned int>", &jetAK8_ncand);
  m_tree->Branch("jetAK8_id", "std::vector<unsigned int>", &jetAK8_id);
  if (is_MC_)
  {
    m_tree->Branch("jetAK8_hflav", "std::vector<unsigned int>", &jetAK8_hflav);
    m_tree->Branch("jetAK8_pflav", "std::vector<int>", &jetAK8_pflav);
    m_tree->Branch("jetAK8_nbhad", "std::vector<unsigned int>", &jetAK8_nbhad);
    m_tree->Branch("jetAK8_nchad", "std::vector<unsigned int>", &jetAK8_nchad);
  }
  m_tree->Branch("jetAK8_nSubjet", "std::vector<int>", &jetAK8_nSubjet);
  m_tree->Branch("jetAK8_softdrop_pt", "std::vector<float>", &jetAK8_softdrop_pt);
  m_tree->Branch("jetAK8_softdrop_eta", "std::vector<float>", &jetAK8_softdrop_eta);
  m_tree->Branch("jetAK8_softdrop_phi", "std::vector<float>", &jetAK8_softdrop_phi);
  m_tree->Branch("jetAK8_softdrop_mass", "std::vector<float>", &jetAK8_softdrop_mass);
  m_tree->Branch("jetAK8_softdrop_pt_raw", "std::vector<float>", &jetAK8_softdrop_pt_raw);
  m_tree->Branch("jetAK8_softdrop_mass_raw", "std::vector<float>", &jetAK8_softdrop_mass_raw);
  // old PNet
  m_tree->Branch("jetAK8_pnet_pfMassDecorrelatedParticleNetprobXbb", "std::vector<float>", &jetAK8_pnet_pfMassDecorrelatedParticleNetprobXbb);
  m_tree->Branch("jetAK8_pnet_pfMassDecorrelatedParticleNetprobQCDbb", "std::vector<float>", &jetAK8_pnet_pfMassDecorrelatedParticleNetprobQCDbb);
  m_tree->Branch("jetAK8_pnet_pfMassDecorrelatedParticleNetprobQCDcc", "std::vector<float>", &jetAK8_pnet_pfMassDecorrelatedParticleNetprobQCDcc);
  m_tree->Branch("jetAK8_pnet_pfMassDecorrelatedParticleNetprobQCDb", "std::vector<float>", &jetAK8_pnet_pfMassDecorrelatedParticleNetprobQCDb);
  m_tree->Branch("jetAK8_pnet_pfMassDecorrelatedParticleNetprobQCDc", "std::vector<float>", &jetAK8_pnet_pfMassDecorrelatedParticleNetprobQCDc);
  m_tree->Branch("jetAK8_pnet_pfMassDecorrelatedParticleNetQCDothers", "std::vector<float>", &jetAK8_pnet_pfMassDecorrelatedParticleNetQCDothers);
  m_tree->Branch("jetAK8_pnet_pfMassDecorrelatedParticleNetXbbvsQCD", "std::vector<float>", &jetAK8_pnet_pfMassDecorrelatedParticleNetXbbvsQCD);
  m_tree->Branch("jetAK8_pnet_pfMassDecorrelatedParticleNetXccvsQCD", "std::vector<float>", &jetAK8_pnet_pfMassDecorrelatedParticleNetXccvsQCD);
  m_tree->Branch("jetAK8_pnet_pfMassDecorrelatedParticleNetXqqvsQCD", "std::vector<float>", &jetAK8_pnet_pfMassDecorrelatedParticleNetXqqvsQCD);
  m_tree->Branch("jetAK8_pnet_pfDecorrelatedParticleNetTvsQCD", "std::vector<float>", &jetAK8_pnet_pfDecorrelatedParticleNetTvsQCD);
  m_tree->Branch("jetAK8_pnet_pfDecorrelatedParticleNetWvsQCD", "std::vector<float>", &jetAK8_pnet_pfDecorrelatedParticleNetWvsQCD);
  m_tree->Branch("jetAK8_pnet_pfDecorrelatedParticleNetZvsQCD", "std::vector<float>", &jetAK8_pnet_pfDecorrelatedParticleNetZvsQCD);

  m_tree->Branch("njetak15", &njetak15);
  m_tree->Branch("jetAK15_pt", "std::vector<float>", &ak15_pt_);
  m_tree->Branch("jetAK15_eta", "std::vector<float>", &ak15_eta_);
  m_tree->Branch("jetAK15_phi", "std::vector<float>", &ak15_phi_);
  m_tree->Branch("jetAK15_mass", "std::vector<float>", &ak15_mass_);
  m_tree->Branch("jetAK15_msoftdrop", "std::vector<float>", &ak15_msoftdrop_);
  m_tree->Branch("jetAK15_area", "std::vector<float>", &ak15_radius_);
  m_tree->Branch("jetAK15_nSubjet", "std::vector<int>", &ak15_nSubjet);
  m_tree->Branch("jetAK15_ncand", "std::vector<unsigned int>", &ak15_ncand);

  if (is_MC_)
  {
    m_tree->Branch("jetAK15_hflav", "std::vector<unsigned int>", &ak15_hflav);
    m_tree->Branch("jetAK15_pflav", "std::vector<int>", &ak15_pflav);
    m_tree->Branch("jetAK15_nbhad", "std::vector<unsigned int>", &ak15_nbhad);
    m_tree->Branch("jetAK15_nchad", "std::vector<unsigned int>", &ak15_nchad);
  }
  m_tree->Branch("jetAK15_pnet_pfMassDecorrelatedParticleNetprobXbb", "std::vector<float>", &ak15_probXbb_);
  m_tree->Branch("jetAK15_pnet_pfMassDecorrelatedParticleNetprobXqq", "std::vector<float>", &ak15_probXqq_);
  m_tree->Branch("jetAK15_pnet_pfMassDecorrelatedParticleNetprobXcc", "std::vector<float>", &ak15_probXcc_);
  m_tree->Branch("jetAK15_pnet_pfMassDecorrelatedParticleNetprobQCD", "std::vector<float>", &ak15_probQCD_);
  m_tree->Branch("jetAK15_pnet_pfMassDecorrelatedParticleNetXbbvsQCD", "std::vector<float>", &ak15_xbbvsqcd_);
  m_tree->Branch("jetAK15_pnet_pfParticleNetMass", "std::vector<float>", &ak15_pnet_mass_);
  m_tree->Branch("jetAK15_glopart_probHbb", "std::vector<float>", &ak15_glopart_probHbb);
  m_tree->Branch("jetAK15_glopart_probHcc", "std::vector<float>", &ak15_glopart_probHcc);
  m_tree->Branch("jetAK15_glopart_probHcs", "std::vector<float>", &ak15_glopart_probHcs);
  m_tree->Branch("jetAK15_glopart_probHqq", "std::vector<float>", &ak15_glopart_probHqq);
  m_tree->Branch("jetAK15_glopart_probHthth", "std::vector<float>", &ak15_glopart_probHthth);
  m_tree->Branch("jetAK15_glopart_probTopbWqq", "std::vector<float>", &ak15_glopart_probTopbWqq);
  m_tree->Branch("jetAK15_glopart_probTopbWq", "std::vector<float>", &ak15_glopart_probTopbWq);
  m_tree->Branch("jetAK15_glopart_probQCD", "std::vector<float>", &ak15_glopart_probQCD);
  m_tree->Branch("jetAK15_glopart_massCorr", "std::vector<float>", &ak15_glopart_massCorr);
  m_tree->Branch("jetAK15_glopart_massCorrGen", "std::vector<float>", &ak15_glopart_massCorrGen);

  m_tree->Branch("nL3AK8jet1", &nL3AK8jet1);
  m_tree->Branch("L3AK8jet1_pt", &L3AK8jet1_pt);
  m_tree->Branch("L3AK8jet1_eta", &L3AK8jet1_eta);
  m_tree->Branch("L3AK8jet1_phi", &L3AK8jet1_phi);
  m_tree->Branch("L3AK8jet1_en", &L3AK8jet1_en);

  m_tree->Branch("nL3AK8jet2", &nL3AK8jet2);
  m_tree->Branch("L3AK8jet2_pt", &L3AK8jet2_pt);
  m_tree->Branch("L3AK8jet2_eta", &L3AK8jet2_eta);
  m_tree->Branch("L3AK8jet2_phi", &L3AK8jet2_phi);
  m_tree->Branch("L3AK8jet2_en", &L3AK8jet2_en);
  /*
     m_tree-> Branch ("nL3AK8jet3", &nL3AK8jet3);
     m_tree-> Branch ("L3AK8jet3_pt", &L3AK8jet3_pt);
     m_tree-> Branch ("L3AK8jet3_eta", &L3AK8jet3_eta);
     m_tree-> Branch ("L3AK8jet3_phi", &L3AK8jet3_phi);
     m_tree-> Branch ("L3AK8jet3_en", &L3AK8jet3_en);
  */
  m_tree->Branch("MET_pt", &MET_pt);
  m_tree->Branch("MET_phi", &MET_phi);
  m_tree->Branch("HLT_IsoMu50_AK8PFJet230_SoftDropMass40", &HLT_IsoMu50_AK8PFJet230_SoftDropMass40);
  m_tree->Branch("HLT_IsoMu50_AK8PFJet230_SoftDropMass40_PNetBB0p06", &HLT_IsoMu50_AK8PFJet230_SoftDropMass40_PNetBB0p06);
  m_tree->Branch("HLT_IsoMu50_AK8PFJet230_SoftDropMass40_PNetBB0p10", &HLT_IsoMu50_AK8PFJet230_SoftDropMass40_PNetBB0p10);
  m_tree->Branch("HLT_AK8PFJet230_SoftDropMass40_PNetBB0p06", &HLT_AK8PFJet230_SoftDropMass40_PNetBB0p06);
  m_tree->Branch("HLT_AK8PFJet230_SoftDropMass40_PNetBB0p10", &HLT_AK8PFJet230_SoftDropMass40_PNetBB0p10);
  m_tree->Branch("HLT_AK8PFJet230_SoftDropMass40_PNetTauTau0p03", &HLT_AK8PFJet230_SoftDropMass40_PNetTauTau0p03);
  m_tree->Branch("HLT_AK8PFJet230_SoftDropMass40_PNetTauTau0p05", &HLT_AK8PFJet230_SoftDropMass40_PNetTauTau0p05);
  m_tree->Branch("HLT_AK8PFJet230_SoftDropMass40_PNetV02BB0p14", &HLT_AK8PFJet230_SoftDropMass40_PNetV02BB0p14);
  m_tree->Branch("HLT_AK8PFJet230_SoftDropMass40_PNetV02BB0p26", &HLT_AK8PFJet230_SoftDropMass40_PNetV02BB0p26);
  m_tree->Branch("HLT_AK15PFJet175_SoftDropMass40", &HLT_AK15PFJet175_SoftDropMass40);
  m_tree->Branch("HLT_AK15PFJet175_SoftDropMass40_GloParTBB0p125", &HLT_AK15PFJet175_SoftDropMass40_GloParTBB0p125);
  m_tree->Branch("HLT_AK15PFJet175_SoftDropMass40_GloParTCC0p25", &HLT_AK15PFJet175_SoftDropMass40_GloParTCC0p25);
}

boostedhbb::~boostedhbb()
{

  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)
}

void boostedhbb::analyze(const edm::Event &iEvent, const edm::EventSetup &iSetup)
{
  edm::Handle<edm::View<pat::Jet>> recjets;
  iEvent.getByToken(tok_jet_, recjets);

  edm::Handle<edm::View<pat::Jet>> pfjetAK8s;
  iEvent.getByToken(tok_pfjetAK8s_, pfjetAK8s);

  edm::Handle<edm::View<pat::Electron>> electrons;
  iEvent.getByToken(tok_electrons_, electrons);

  edm::Handle<double> rhoInfo;
  iEvent.getByToken(rhoToken, rhoInfo);

  edm::Handle<std::vector<reco::Vertex>> vertices;
  iEvent.getByToken(tok_primaryVertices_, vertices);

  edm::Handle<pat::MuonCollection> muons;
  iEvent.getByToken(tok_muons_, muons);

  edm::Handle<reco::VertexCollection> primaryVertices;
  iEvent.getByToken(tok_primaryVertices_, primaryVertices);

  edm::Handle<edm::TriggerResults> METFilterResults;
  iEvent.getByToken(metfilterspatLabel_, METFilterResults);
  if (!(METFilterResults.isValid()))
    iEvent.getByToken(metfiltersrecoLabel_, METFilterResults);

  // rho index
  edm::Handle<double> Rho_PF;
  iEvent.getByToken(rhoToken, Rho_PF);
  rho = *Rho_PF;

  reco::Vertex vertex;
  if (primaryVertices.isValid())
  {
    if (primaryVertices->size() > 0)
    {
      vertex = primaryVertices->at(0);
    }
  }
  // Number of primary vertices
  edm::Handle<std::vector<reco::Vertex>> theVertices;
  iEvent.getByToken(verticesToken_, theVertices);
  nPV = theVertices->size();
  // Variables related to the MC only
  if (is_MC_)
  {
    edm::Handle<GenEventInfoProduct> genEvtInfo;
    iEvent.getByToken(genInfoToken_, genEvtInfo);
    _genWeight = genEvtInfo->weight();
    edm::Handle<std::vector<PileupSummaryInfo>> PupInfo;
    iEvent.getByToken(pileupToken, PupInfo);
    std::vector<PileupSummaryInfo>::const_iterator PVI;
    nPU = -1;
    for (PVI = PupInfo->begin(); PVI != PupInfo->end(); ++PVI)
    {
      int BX = PVI->getBunchCrossing();
      if (BX == 0)
      {
        nPU = PVI->getTrueNumInteractions();
        continue;
      }
    }
    m_tree2->Fill();
  }
  else
  {
    _genWeight = 1.0;
    nPU = 1.0;
  }
  // m_tree2->Fill();

  edm::Handle<edm::TriggerResults> HLTR;
  iEvent.getByToken(HLTriggerResults_, HLTR);

  if (!HLTR.isValid())
  {
    edm::LogWarning("boostedhbb") << "TriggerResults handle is invalid";
    return;
  }

  const edm::TriggerNames &triggerNames_ = iEvent.triggerNames(*HLTR);

  const std::string pathmu = resolveHLTPath(triggerNames_, "HLT_IsoMu24_v");
  const std::string pathcont1 = resolveHLTPath(triggerNames_, "HLT_IsoMu50_AK8PFJet230_SoftDropMass40_v");
  const std::string pathcont2 = resolveHLTPath(triggerNames_, "HLT_IsoMu50_AK8PFJet230_SoftDropMass40_PNetBB0p06_v");
  const std::string pathcont3 = resolveHLTPath(triggerNames_, "HLT_IsoMu50_AK8PFJet230_SoftDropMass40_PNetBB0p10_v");
  const std::string pathcont4 = resolveHLTPath(triggerNames_, "HLT_AK8PFJet230_SoftDropMass40_PNetBB0p06_v");
  const std::string pathcont5 = resolveHLTPath(triggerNames_, "HLT_AK8PFJet230_SoftDropMass40_PNetBB0p10_v");
  const std::string pathcont6 = resolveHLTPath(triggerNames_, "HLT_AK8PFJet230_SoftDropMass40_PNetTauTau0p03_v");
  const std::string pathcont7 = resolveHLTPath(triggerNames_, "HLT_AK8PFJet230_SoftDropMass40_PNetTauTau0p05_v");
  const std::string pathcont8 = resolveHLTPath(triggerNames_, "HLT_AK8PFJet230_SoftDropMass40_PNetV02BB0p14_v");
  const std::string pathcont9 = resolveHLTPath(triggerNames_, "HLT_AK8PFJet230_SoftDropMass40_PNetV02BB0p26_v");
  const std::string pathcont10 = resolveHLTPath(triggerNames_, "HLT_AK15PFJet175_SoftDropMass40_v");
  const std::string pathcont11 = resolveHLTPath(triggerNames_, "HLT_AK15PFJet175_SoftDropMass40_GloParTBB0p125_v");
  const std::string pathcont12 = resolveHLTPath(triggerNames_, "HLT_AK15PFJet175_SoftDropMass40_GloParTCC0p25_v");

  const Bool_t iscont1 = safeAccept(HLTR, triggerNames_, pathcont1);
  const Bool_t iscont2 = safeAccept(HLTR, triggerNames_, pathcont2);
  const Bool_t iscont3 = safeAccept(HLTR, triggerNames_, pathcont3);
  const Bool_t iscont4 = safeAccept(HLTR, triggerNames_, pathcont4);
  const Bool_t iscont5 = safeAccept(HLTR, triggerNames_, pathcont5);
  const Bool_t iscont6 = safeAccept(HLTR, triggerNames_, pathcont6);
  const Bool_t iscont7 = safeAccept(HLTR, triggerNames_, pathcont7);
  const Bool_t iscont8 = safeAccept(HLTR, triggerNames_, pathcont8);
  const Bool_t iscont9 = safeAccept(HLTR, triggerNames_, pathcont9);
  const Bool_t iscont10 = safeAccept(HLTR, triggerNames_, pathcont10);
  const Bool_t iscont11 = safeAccept(HLTR, triggerNames_, pathcont11);
  const Bool_t iscont12 = safeAccept(HLTR, triggerNames_, pathcont12);

  const Bool_t passMuTrig = safeAccept(HLTR, triggerNames_, pathmu);

  // muon-eg trigger selection.
  if (passMuTrig)
  {
    // Event informations
    _run = iEvent.id().run();
    _event = iEvent.id().event();
    _lumi = iEvent.luminosityBlock();
    // MET metfilters
    const edm::TriggerNames &metfilterName = iEvent.triggerNames(*METFilterResults);
    unsigned int goodVerticesIndex_ = metfilterName.triggerIndex("Flag_goodVertices");
    Flag_goodVertices_ = METFilterResults.product()->accept(goodVerticesIndex_);
    unsigned int globalSuperTightHalo2016FilterIndex_ = metfilterName.triggerIndex("Flag_globalSuperTightHalo2016Filter");
    Flag_globalSuperTightHalo2016Filter_ = METFilterResults.product()->accept(globalSuperTightHalo2016FilterIndex_);
    unsigned int EcalDeadCellTriggerPrimitiveFilterIndex_ = metfilterName.triggerIndex("Flag_EcalDeadCellTriggerPrimitiveFilter");
    Flag_EcalDeadCellTriggerPrimitiveFilter_ = METFilterResults.product()->accept(EcalDeadCellTriggerPrimitiveFilterIndex_);
    unsigned int BadPFMuonFilterIndex_ = metfilterName.triggerIndex("Flag_BadPFMuonFilter");
    Flag_BadPFMuonFilter_ = METFilterResults.product()->accept(BadPFMuonFilterIndex_);
    unsigned int BadPFMuonFilterDzIndex_ = metfilterName.triggerIndex("Flag_BadPFMuonDzFilter");
    Flag_BadPFMuonDzFilter_ = METFilterResults.product()->accept(BadPFMuonFilterDzIndex_);
    unsigned int hfNoisyHitsIndex_ = metfilterName.triggerIndex("Flag_hfNoisyHitsFilter");
    Flag_hfNoisyHitsFilter_ = METFilterResults.product()->accept(hfNoisyHitsIndex_);
    unsigned int eeBadScFilterIndex_ = metfilterName.triggerIndex("Flag_eeBadScFilter");
    Flag_eeBadScFilter_ = METFilterResults.product()->accept(eeBadScFilterIndex_);
    unsigned int ecalBadCalibFilterIndex_ = metfilterName.triggerIndex("Flag_ecalBadCalibFilter");
    Flag_ecalBadCalibFilter_ = METFilterResults.product()->accept(ecalBadCalibFilterIndex_);
    // PUPPI MET

    MET_pt = -999.f;
    MET_phi = -999.f;

    edm::Handle<std::vector<pat::MET>> ThePUPPIMET;
    iEvent.getByToken(puppimetToken_, ThePUPPIMET);
    if (ThePUPPIMET.isValid() && !ThePUPPIMET->empty())
    {
      const pat::MET &puppimet = ThePUPPIMET->front();
      MET_pt = puppimet.pt();
      MET_phi = puppimet.phi();
    }

    //**********************************************************//
    // electron selection
    vector<double> ele_pt;
    vector<double> ele_eta;
    vector<double> ele_phi;
    vector<double> ele_en;
    vector<double> ele_id_T;
    vector<double> ele_id_L;
    vector<int> ele_ch;
    vector<double> ele_dxy;
    vector<double> ele_dz;
    for (const pat::Electron &el : *electrons)
    {
      // std::cout << el.pt() << "       " << el.eta() << std::endl;
      if (el.pt() < 10 || fabs(el.eta()) > 2.5)
        continue;
      reco::GsfTrackRef gsftrk1 = el.gsfTrack();
      Float_t el_dxy_ = gsftrk1->dxy(vertex.position());
      Float_t el_dz_ = gsftrk1->dz(vertex.position());
      // storing the electron informations
      ele_dxy.push_back(el_dxy_);
      ele_dz.push_back(el_dz_);
      ele_pt.push_back(el.pt());
      ele_eta.push_back(el.eta());
      ele_phi.push_back(el.phi());
      ele_en.push_back(el.energy());
      ele_ch.push_back(el.charge());
      ele_id_T.push_back(el.electronID("mvaEleID-Fall17-iso-V2-wp80"));
      ele_id_L.push_back(el.electronID("cutBasedElectronID-RunIIIWinter22-V1-veto"));
      // std::cout <<  "electron id" <<  el.electronID("mvaEleID-Fall17-iso-V2-wp80") << "	" << el.electronID("mvaEleID-Fall17-iso-V2-wp90") << std::endl;
    }
    // veto of loose electron
    bool extraele = false;
    for (int pp = 0; pp < int(ele_pt.size()); pp++)
    {
      if (ele_id_L.at(pp))
      {
        if ((ele_eta.at(pp) < 1.445 && fabs(ele_dxy.at(pp)) < (dxy_cut) && fabs(ele_dz.at(pp)) < (dz_cut)) || (ele_eta.at(pp) > 1.445 && fabs(ele_dxy.at(pp)) < (2.0 * dxy_cut) && fabs(ele_dz.at(pp)) < (2.0 * dz_cut)))
        {
          extraele = true;
          break;
        }
      }
    }

    // std::cout << "The electron status :  good electron "  << isGDele << " | loose electron " << extraele << std::endl;
    //**********************************************************//

    // muon selection
    vector<double> mu_pt;
    vector<double> mu_eta;
    vector<double> mu_phi;
    vector<double> mu_en;
    vector<double> mu_id_T;
    vector<double> mu_id_L;
    vector<double> mu_iso;
    vector<double> mu_dxy;
    vector<double> mu_dz;
    vector<int> mu_ch;
    for (const pat::Muon &mu : *muons)
    {
      // std::cout << mu.pt() << "	" << mu.eta() << std::endl;
      if (mu.pt() < 10 || fabs(mu.eta()) > 2.4)
        continue;
      Float_t mu_dxy_ = mu.muonBestTrack()->dxy(vertex.position());
      ;
      Float_t mu_dz_ = mu.muonBestTrack()->dz(vertex.position());
      ;
      Float_t Muon_pfiso = (mu.pfIsolationR04().sumChargedHadronPt + max(0., mu.pfIsolationR04().sumNeutralHadronEt + mu.pfIsolationR04().sumPhotonEt - 0.5 * mu.pfIsolationR04().sumPUPt)) / mu.pt();
      // storing the muon informations
      mu_dxy.push_back(mu_dxy_);
      mu_dz.push_back(mu_dz_);
      mu_iso.push_back(Muon_pfiso);
      mu_pt.push_back(mu.pt());
      mu_eta.push_back(mu.eta());
      mu_phi.push_back(mu.phi());
      mu_en.push_back(mu.energy());
      mu_ch.push_back(mu.charge());
      mu_id_T.push_back(mu.passed(reco::Muon::CutBasedIdTight));
      mu_id_L.push_back(mu.passed(reco::Muon::CutBasedIdLoose));
      // std::cout <<  "muon id" <<  mu.passed(reco::Muon::CutBasedIdLoose) << "       " << mu.passed(reco::Muon::CutBasedIdTight) << std::endl;
    }
    // std::cout << "Total Muons found : " << int(mu_pt.size()) <<  std::endl;
    // leptons are chosen to be opposite in charge
    muon_pt = -999.9, muon_eta = -999.9, muon_phi = -999.9, muon_energy = -999.9;
    muon_charge = 0;
    int good_mu_ord = 999;
    bool isGDmu = false;
    bool extramu = false;

    for (int pp = 0; pp < int(mu_pt.size()); pp++)
    {
      if (mu_pt.at(pp) > 30 && mu_id_T.at(pp) && mu_iso.at(pp) < 0.15 && fabs(mu_dxy.at(pp)) < 0.2 && fabs(mu_dz.at(pp)) < 0.5)
      {
        muon_pt = mu_pt.at(pp);
        muon_eta = mu_eta.at(pp);
        muon_phi = mu_phi.at(pp);
        muon_energy = mu_en.at(pp);
        muon_charge = mu_ch.at(pp);
        good_mu_ord = pp;
        isGDmu = true;
        break;
      }
    }

    // veto of loose electron
    for (int pp = 0; pp < int(mu_pt.size()); pp++)
    {
      if (pp != good_mu_ord && mu_id_L.at(pp) && mu_iso.at(pp) < 0.25 && fabs(mu_dxy.at(pp)) < 0.2 && fabs(mu_dz.at(pp)) < 0.5)
      {
        extramu = true;
        break;
      }
    }
    // std::cout << "The mu status :  good muon "  << isGDmu << " | loose muon " << extramu << std::endl;
    //**********************************************************//
    // The jet selection
    jet_pt.clear();
    jet_eta.clear();
    jet_phi.clear();
    jet_en.clear();
    jet_mass.clear();
    jet_id.clear();
    jet_PFBTG.clear();
    jet_pflv.clear();
    jet_hflv.clear();
    njet = 0;
    HT = 0.0;
    for (auto jet = recjets->begin(); jet != recjets->end(); jet++)
    {
      if (jet->pt() < 25 || fabs(jet->eta()) > 2.5)
        continue;
      TLorentzVector recojet;
      recojet.SetPtEtaPhiM(jet->pt(), jet->eta(), jet->phi(), jet->mass());
      JetIDVars AK4idvars;

      AK4idvars.NHF = jet->neutralHadronEnergyFraction();
      AK4idvars.NEMF = jet->neutralEmEnergyFraction();
      AK4idvars.MUF = jet->muonEnergyFraction();
      AK4idvars.CHF = jet->chargedHadronEnergyFraction();
      AK4idvars.CEMF = jet->chargedEmEnergyFraction();
      AK4idvars.NumConst = (jet->chargedMultiplicity() + jet->neutralMultiplicity());
      AK4idvars.NumNeutralParticle = jet->neutralMultiplicity();
      AK4idvars.CHM = jet->chargedHadronMultiplicity();

      Bool_t jet_id_ = getJetID(AK4idvars, "PUPPI", "2022FG", jet->eta());
      Float_t jet_btag = jet->bDiscriminator("pfParticleNetFromMiniAODAK4PuppiCentralDiscriminatorsJetTags:BvsAll");
      jet_pt.push_back(jet->pt());
      jet_eta.push_back(jet->eta());
      jet_phi.push_back(jet->phi());
      jet_en.push_back(jet->energy());
      jet_mass.push_back(jet->mass());
      jet_PFBTG.push_back(jet_btag);
      jet_id.push_back(jet_id_);
      if (is_MC_)
      {
        jet_pflv.push_back(jet->hadronFlavour());
        jet_hflv.push_back(jet->partonFlavour());
      }
      HT = HT + jet->pt();
      njet++;
    } // jet

    // AK-8 jet collection
    njetak8 = 0;
    jetAK8_pt.clear();
    jetAK8_eta.clear();
    jetAK8_phi.clear();
    jetAK8_mass.clear();
    jetAK8_pt_raw.clear();
    jetAK8_mass_raw.clear();
    jetAK8_area.clear();
    jetAK8_pnet_probHbb.clear();
    jetAK8_pnet_probHcc.clear();
    jetAK8_pnet_probHqq.clear();
    jetAK8_pnet_probHgg.clear();
    jetAK8_pnet_probHtt.clear();
    jetAK8_pnet_probHtm.clear();
    jetAK8_pnet_probHte.clear();
    jetAK8_pnet_probQCD2HF.clear();
    jetAK8_pnet_probQCD1HF.clear();
    jetAK8_pnet_probQCD0HF.clear();
    jetAK8_pnet_HbbVsQCD.clear();
    jetAK8_pnet_mass.clear();
    jetAK8_id.clear();
    jetAK8_ncand.clear();
    jetAK8_hflav.clear();
    jetAK8_pflav.clear();
    jetAK8_nbhad.clear();
    jetAK8_nchad.clear();
    jetAK8_softdrop_pt.clear();
    jetAK8_softdrop_eta.clear();
    jetAK8_softdrop_phi.clear();
    jetAK8_softdrop_mass.clear();
    jetAK8_softdrop_pt_raw.clear();
    jetAK8_softdrop_mass_raw.clear();
    jetAK8_softdrop_subjet_pt.clear();
    jetAK8_softdrop_subjet_pt_raw.clear();
    jetAK8_softdrop_subjet_eta.clear();
    jetAK8_softdrop_subjet_phi.clear();
    jetAK8_softdrop_subjet_mass.clear();
    jetAK8_softdrop_subjet_mass_raw.clear();
    jetAK8_pnet_pfMassDecorrelatedParticleNetprobXbb.clear();
    jetAK8_pnet_pfMassDecorrelatedParticleNetprobQCDbb.clear();
    jetAK8_pnet_pfMassDecorrelatedParticleNetprobQCDcc.clear();
    jetAK8_pnet_pfMassDecorrelatedParticleNetprobQCDb.clear();
    jetAK8_pnet_pfMassDecorrelatedParticleNetprobQCDc.clear();
    jetAK8_pnet_pfMassDecorrelatedParticleNetQCDothers.clear();
    jetAK8_pnet_pfMassDecorrelatedParticleNetXbbvsQCD.clear();
    jetAK8_pnet_pfMassDecorrelatedParticleNetXccvsQCD.clear();
    jetAK8_pnet_pfMassDecorrelatedParticleNetXqqvsQCD.clear();
    jetAK8_pnet_pfDecorrelatedParticleNetTvsQCD.clear();
    jetAK8_pnet_pfDecorrelatedParticleNetWvsQCD.clear();
    jetAK8_pnet_pfDecorrelatedParticleNetZvsQCD.clear();
    jetAK8_nSubjet.clear();
    for (auto jetAK8 = pfjetAK8s->begin(); jetAK8 != pfjetAK8s->end(); jetAK8++)
    {
      if (jetAK8->pt() < 200.0 || fabs(jetAK8->eta()) > 2.5)
        continue;
      jetAK8_pt.push_back(jetAK8->pt());
      jetAK8_eta.push_back(jetAK8->eta());
      jetAK8_phi.push_back(jetAK8->phi());
      jetAK8_mass.push_back(jetAK8->mass());
      jetAK8_pt_raw.push_back(jetAK8->correctedJet("Uncorrected").pt());
      jetAK8_mass_raw.push_back(jetAK8->correctedJet("Uncorrected").mass());
      jetAK8_pnet_probHbb.push_back(jetAK8->bDiscriminator("pfParticleNetFromMiniAODAK8JetTags:probHbb"));
      jetAK8_pnet_probHcc.push_back(jetAK8->bDiscriminator("pfParticleNetFromMiniAODAK8JetTags:probHcc"));
      jetAK8_pnet_probHqq.push_back(jetAK8->bDiscriminator("pfParticleNetFromMiniAODAK8JetTags:probHqq"));
      jetAK8_pnet_probHgg.push_back(jetAK8->bDiscriminator("pfParticleNetFromMiniAODAK8JetTags:probHgg"));
      jetAK8_pnet_probHtt.push_back(jetAK8->bDiscriminator("pfParticleNetFromMiniAODAK8JetTags:probHtt"));
      jetAK8_pnet_probHtm.push_back(jetAK8->bDiscriminator("pfParticleNetFromMiniAODAK8JetTags:probHtm"));
      jetAK8_pnet_probHte.push_back(jetAK8->bDiscriminator("pfParticleNetFromMiniAODAK8JetTags:probHte"));
      jetAK8_pnet_probQCD2HF.push_back(jetAK8->bDiscriminator("pfParticleNetFromMiniAODAK8JetTags:probQCD2hf"));
      jetAK8_pnet_probQCD1HF.push_back(jetAK8->bDiscriminator("pfParticleNetFromMiniAODAK8JetTags:probQCD1hf"));
      jetAK8_pnet_probQCD0HF.push_back(jetAK8->bDiscriminator("pfParticleNetFromMiniAODAK8JetTags:probQCD0hf"));
      jetAK8_pnet_HbbVsQCD.push_back(jetAK8->bDiscriminator("pfParticleNetFromMiniAODAK8DiscriminatorsJetTags:HbbvsQCD"));
      jetAK8_pnet_mass.push_back(jetAK8->bDiscriminator("pfParticleNetFromMiniAODAK8JetTags:masscorr") * jetAK8->mass());
      jetAK8_pnet_pfMassDecorrelatedParticleNetprobXbb.push_back(jetAK8->bDiscriminator("pfMassDecorrelatedParticleNetJetTags:probXbb"));
      jetAK8_pnet_pfMassDecorrelatedParticleNetprobQCDbb.push_back(jetAK8->bDiscriminator("pfMassDecorrelatedParticleNetJetTags:probQCDbb"));
      jetAK8_pnet_pfMassDecorrelatedParticleNetprobQCDcc.push_back(jetAK8->bDiscriminator("pfMassDecorrelatedParticleNetJetTags:probQCDcc"));
      jetAK8_pnet_pfMassDecorrelatedParticleNetprobQCDb.push_back(jetAK8->bDiscriminator("pfMassDecorrelatedParticleNetJetTags:probQCDb"));
      jetAK8_pnet_pfMassDecorrelatedParticleNetprobQCDc.push_back(jetAK8->bDiscriminator("pfMassDecorrelatedParticleNetJetTags:probQCDc"));
      jetAK8_pnet_pfMassDecorrelatedParticleNetQCDothers.push_back(jetAK8->bDiscriminator("pfMassDecorrelatedParticleNetJetTags:probQCDothers"));
      jetAK8_pnet_pfMassDecorrelatedParticleNetXbbvsQCD.push_back(jetAK8->bDiscriminator("pfMassDecorrelatedParticleNetDiscriminatorsJetTags:XbbvsQCD"));
      jetAK8_pnet_pfMassDecorrelatedParticleNetXccvsQCD.push_back(jetAK8->bDiscriminator("pfMassDecorrelatedParticleNetDiscriminatorsJetTags:XccvsQCD"));
      jetAK8_pnet_pfMassDecorrelatedParticleNetXqqvsQCD.push_back(jetAK8->bDiscriminator("pfMassDecorrelatedParticleNetDiscriminatorsJetTags:XqqvsQCD"));
      jetAK8_pnet_pfDecorrelatedParticleNetTvsQCD.push_back(jetAK8->bDiscriminator("pfParticleNetDiscriminatorsJetTags:TvsQCD"));
      jetAK8_pnet_pfDecorrelatedParticleNetWvsQCD.push_back(jetAK8->bDiscriminator("pfParticleNetDiscriminatorsJetTags:WvsQCD"));
      jetAK8_pnet_pfDecorrelatedParticleNetZvsQCD.push_back(jetAK8->bDiscriminator("pfParticleNetDiscriminatorsJetTags:ZvsQCD"));

      if (is_MC_)
      {
        jetAK8_nbhad.push_back(jetAK8->jetFlavourInfo().getbHadrons().size());
        jetAK8_nchad.push_back(jetAK8->jetFlavourInfo().getcHadrons().size());
        jetAK8_hflav.push_back(jetAK8->hadronFlavour());
        jetAK8_pflav.push_back(jetAK8->partonFlavour());
      }
      jetAK8_ncand.push_back(jetAK8->chargedHadronMultiplicity() + jetAK8->neutralHadronMultiplicity() + jetAK8->electronMultiplicity() + jetAK8->photonMultiplicity() + jetAK8->muonMultiplicity());

      JetIDVars AK8idvars;
      AK8idvars.NHF = jetAK8->neutralHadronEnergyFraction();
      AK8idvars.NEMF = jetAK8->neutralEmEnergyFraction();
      AK8idvars.MUF = jetAK8->muonEnergyFraction();
      AK8idvars.CHF = jetAK8->chargedHadronEnergyFraction();
      AK8idvars.CEMF = jetAK8->chargedEmEnergyFraction();
      AK8idvars.NumConst = (jetAK8->chargedMultiplicity() + jetAK8->neutralMultiplicity());
      AK8idvars.NumNeutralParticle = jetAK8->neutralMultiplicity();
      AK8idvars.CHM = jetAK8->chargedHadronMultiplicity();

      Bool_t jetAK8_id_ = getJetID(AK8idvars, "PUPPI", "2022FG", jetAK8->eta());
      jetAK8_id.push_back(jetAK8_id_);
      // subjet collection
      TLorentzVector jetAK8_softdrop_4V;
      TLorentzVector jetAK8_softdrop_4V_raw;
      pat::JetPtrCollection subjets;
      if (jetAK8->nSubjetCollections() > 0)
      {
        subjets = jetAK8->subjets(subJetCollectionName);
        // std::sort(subjets.begin(), subjets.end(), jetPtrSorter);
        // std::vector<unsigned int> subjetToSkip;
        for (size_t isub = 0; isub < subjets.size(); isub++)
        {
          jetAK8_softdrop_subjet_pt.push_back(subjets.at(isub)->pt());
          jetAK8_softdrop_subjet_pt_raw.push_back(subjets.at(isub)->correctedJet("Uncorrected").pt());
          jetAK8_softdrop_subjet_eta.push_back(subjets.at(isub)->eta());
          jetAK8_softdrop_subjet_phi.push_back(subjets.at(isub)->phi());
          jetAK8_softdrop_subjet_mass.push_back(subjets.at(isub)->mass());
          jetAK8_softdrop_subjet_mass_raw.push_back(subjets.at(isub)->correctedJet("Uncorrected").mass());
          TLorentzVector subjet_4v, subjet_raw_4v;
          subjet_4v.SetPtEtaPhiM(subjets.at(isub)->pt(), subjets.at(isub)->eta(), subjets.at(isub)->phi(), subjets.at(isub)->mass());
          subjet_raw_4v.SetPtEtaPhiM(subjets.at(isub)->correctedJet("Uncorrected").pt(), subjets.at(isub)->correctedJet("Uncorrected").eta(),
                                     subjets.at(isub)->correctedJet("Uncorrected").phi(), subjets.at(isub)->correctedJet("Uncorrected").mass());
          jetAK8_softdrop_4V += subjet_4v;
          jetAK8_softdrop_4V_raw += subjet_raw_4v;
        }
      }
      jetAK8_nSubjet.push_back(jetAK8->nSubjetCollections());
      jetAK8_softdrop_pt.push_back(jetAK8_softdrop_4V.Pt());
      jetAK8_softdrop_eta.push_back(jetAK8_softdrop_4V.Eta());
      jetAK8_softdrop_phi.push_back(jetAK8_softdrop_4V.Phi());
      jetAK8_softdrop_mass.push_back(jetAK8_softdrop_4V.M());
      jetAK8_softdrop_pt_raw.push_back(jetAK8_softdrop_4V_raw.Pt());
      jetAK8_softdrop_mass_raw.push_back(jetAK8_softdrop_4V_raw.M());
      jetAK8_area.push_back(jetAK8->jetArea());
      njetak8++;
    }
    ak15_pt_.clear();
    ak15_eta_.clear();
    ak15_phi_.clear();
    ak15_mass_.clear();
    ak15_msoftdrop_.clear();
    ak15_nSubjet.clear();
    ak15_probXbb_.clear();
    ak15_probXqq_.clear();
    ak15_probXcc_.clear();
    ak15_probQCD_.clear();
    ak15_xbbvsqcd_.clear();
    ak15_pnet_mass_.clear();
    ak15_radius_.clear();
    ak15_ncand.clear();
    ak15_hflav.clear();
    ak15_pflav.clear();
    ak15_nbhad.clear();
    ak15_nchad.clear();
    ak15_glopart_probHbb.clear();
    ak15_glopart_probHcc.clear();
    ak15_glopart_probHcs.clear();
    ak15_glopart_probHqq.clear();
    ak15_glopart_probHthth.clear();
    ak15_glopart_probTopbWqq.clear();
    ak15_glopart_probTopbWq.clear();
    ak15_glopart_probQCD.clear();
    ak15_glopart_massCorr.clear();
    ak15_glopart_massCorrGen.clear();

    edm::Handle<std::vector<pat::Jet>> ak15Jets;
    iEvent.getByToken(ak15JetsToken_, ak15Jets);

    njetak15 = 0;
    if (ak15Jets.isValid())
    {
      for (const auto &jet : *ak15Jets)
      {
        ak15_pt_.push_back(jet.pt());
        ak15_eta_.push_back(jet.eta());
        ak15_phi_.push_back(jet.phi());
        ak15_mass_.push_back(jet.mass());
        ak15_msoftdrop_.push_back(jet.groomedMass());
        ak15_nSubjet.push_back(jet.nSubjetCollections());
        if (is_MC_)
        {
          ak15_nbhad.push_back(jet.jetFlavourInfo().getbHadrons().size());
          ak15_nchad.push_back(jet.jetFlavourInfo().getcHadrons().size());
          ak15_hflav.push_back(jet.hadronFlavour());
          ak15_pflav.push_back(jet.partonFlavour());
        }
        ak15_ncand.push_back(jet.chargedHadronMultiplicity() + jet.neutralHadronMultiplicity() + jet.electronMultiplicity() + jet.photonMultiplicity() + jet.muonMultiplicity());

        float probXbb = -99.f;
        float probXqq = -99.f;
        float probXcc = -99.f;
        float probQCDbb = -99.f;
        float probQCDcc = -99.f;
        float probQCDb = -99.f;
        float probQCDc = -99.f;
        float probQCDothers = -99.f;
        float probQCD = -99.f;
        float xbbvsqcd = -99.f;
        probXbb = jet.bDiscriminator("pfMassDecorrelatedParticleNetJetTags:probXbb");
        probXqq = jet.bDiscriminator("pfMassDecorrelatedParticleNetJetTags:probXqq");
        probXcc = jet.bDiscriminator("pfMassDecorrelatedParticleNetJetTags:probXcc");
        probQCDbb = jet.bDiscriminator("pfMassDecorrelatedParticleNetJetTags:probQCDbb");
        probQCDcc = jet.bDiscriminator("pfMassDecorrelatedParticleNetJetTags:probQCDcc");
        probQCDc = jet.bDiscriminator("pfMassDecorrelatedParticleNetJetTags:probQCDc");
        probQCDb = jet.bDiscriminator("pfMassDecorrelatedParticleNetJetTags:probQCDb");
        probQCDothers = jet.bDiscriminator("pfMassDecorrelatedParticleNetJetTags:probQCDothers");
        probQCD = probQCDbb + probQCDcc + probQCDc + probQCDb + probQCDothers;
        xbbvsqcd = probXbb / (probXbb + probQCD);
        ak15_probXbb_.push_back(probXbb);
        ak15_probXcc_.push_back(probXcc);
        ak15_probXqq_.push_back(probXqq);
        ak15_probQCD_.push_back(probQCD);
        ak15_xbbvsqcd_.push_back(xbbvsqcd);
        ak15_pnet_mass_.push_back(jet.bDiscriminator("pfParticleNetMassRegressionJetTags:mass"));
        ak15_radius_.push_back(jet.jetArea());
        // info taken from https://github.com/cms-sw/cmssw/blob/243edc69092488ae4ed876479364b2cefa979658/PhysicsTools/NanoAOD/python/jetsAK8_cff.py#L59-L80
        ak15_glopart_probHbb.push_back(jet.bDiscriminator("pfGlobalParticleTransformerAK15JetTags:probXbb"));
        ak15_glopart_probHcc.push_back(jet.bDiscriminator("pfGlobalParticleTransformerAK15JetTags:probXcc"));
        ak15_glopart_probHcs.push_back(jet.bDiscriminator("pfGlobalParticleTransformerAK15JetTags:probXcs"));
        ak15_glopart_probHqq.push_back(jet.bDiscriminator("pfGlobalParticleTransformerAK15JetTags:probXqq"));
        ak15_glopart_probHthth.push_back(jet.bDiscriminator("pfGlobalParticleTransformerAK15JetTags:probXtauhtauh"));
        ak15_glopart_probTopbWqq.push_back(jet.bDiscriminator("pfGlobalParticleTransformerAK15JetTags:probTopbWqq"));
        ak15_glopart_probTopbWq.push_back(jet.bDiscriminator("pfGlobalParticleTransformerAK15JetTags:probTopbWq"));
        ak15_glopart_probQCD.push_back(jet.bDiscriminator("pfGlobalParticleTransformerAK15JetTags:probQCD"));
        ak15_glopart_massCorr.push_back(jet.bDiscriminator("pfGlobalParticleTransformerAK15JetTags:massCorrX2p"));
        ak15_glopart_massCorrGen.push_back(jet.bDiscriminator("pfGlobalParticleTransformerAK15JetTags:massCorrGeneric"));
        njetak15++;
      }
    }
    nCaloAK8jet = 0;
    nL3AK8jet1 = 0;
    nL3AK8jet2 = 0;
    CaloAK8jet_pt.clear();
    CaloAK8jet_eta.clear();
    CaloAK8jet_phi.clear();
    CaloAK8jet_en.clear();
    L3AK8jet1_pt.clear();
    L3AK8jet1_eta.clear();
    L3AK8jet1_phi.clear();
    L3AK8jet1_en.clear();
    L3AK8jet2_pt.clear();
    L3AK8jet2_eta.clear();
    L3AK8jet2_phi.clear();
    L3AK8jet2_en.clear();

    edm::Handle<std::vector<pat::TriggerObjectStandAlone>> triggerObjects;
    iEvent.getByToken(triggerObjects_, triggerObjects);
    const edm::TriggerNames &names_ = iEvent.triggerNames(*HLTR);
    for (pat::TriggerObjectStandAlone obj : *triggerObjects)
    {
      obj.unpackPathNames(names_);
      if (obj.collection() == "hltAK8CaloJetsCorrectedIDPassed::HLT")
      {
        CaloAK8jet_pt.push_back(obj.pt());
        CaloAK8jet_eta.push_back(obj.eta());
        CaloAK8jet_phi.push_back(obj.phi());
        CaloAK8jet_en.push_back(obj.energy());
        nCaloAK8jet++;
      }
      if (obj.collection() == "hltAK8PFJets230SoftDropMass40::HLT")
      {
        L3AK8jet1_pt.push_back(obj.pt());
        L3AK8jet1_eta.push_back(obj.eta());
        L3AK8jet1_phi.push_back(obj.phi());
        L3AK8jet1_en.push_back(obj.energy());
        nL3AK8jet1++;
      }
      if (obj.collection() == "hltAK8PFSoftDropJets230::HLT")
      {
        L3AK8jet2_pt.push_back(obj.pt());
        L3AK8jet2_eta.push_back(obj.eta());
        L3AK8jet2_phi.push_back(obj.phi());
        L3AK8jet2_en.push_back(obj.energy());
        nL3AK8jet2++;
      }
    }
    // filling the HLT informations in boolean formulation.
    HLT_IsoMu50_AK8PFJet230_SoftDropMass40 = iscont1;
    HLT_IsoMu50_AK8PFJet230_SoftDropMass40_PNetBB0p06 = iscont2;
    HLT_IsoMu50_AK8PFJet230_SoftDropMass40_PNetBB0p10 = iscont3;
    HLT_AK8PFJet230_SoftDropMass40_PNetBB0p06 = iscont4;
    HLT_AK8PFJet230_SoftDropMass40_PNetBB0p10 = iscont5;
    HLT_AK8PFJet230_SoftDropMass40_PNetTauTau0p03 = iscont6;
    HLT_AK8PFJet230_SoftDropMass40_PNetTauTau0p05 = iscont7;
    HLT_AK8PFJet230_SoftDropMass40_PNetV02BB0p14 = iscont8;
    HLT_AK8PFJet230_SoftDropMass40_PNetV02BB0p26 = iscont9;
    HLT_AK15PFJet175_SoftDropMass40 = iscont10;
    HLT_AK15PFJet175_SoftDropMass40_GloParTBB0p125 = iscont11;
    HLT_AK15PFJet175_SoftDropMass40_GloParTCC0p25 = iscont12;
    if (isGDmu && !extramu && !extraele && (njetak8 > 0 || njetak15 > 0))
    {
      m_tree->Fill();
    }

    //**********storing the trigger results******************//
  } // control trigger selection
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void boostedhbb::fillDescriptions(edm::ConfigurationDescriptions &descriptions)
{
  // The following says we do not know what parameters are allowed so do no validation
  //  Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

// define this as a plug-in
DEFINE_FWK_MODULE(boostedhbb);
