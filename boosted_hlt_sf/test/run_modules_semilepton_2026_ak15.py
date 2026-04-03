import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras

process = cms.Process("newPAT")

from PhysicsTools.PatAlgos.tools.helpers import getPatAlgosToolsTask
patAlgosToolsTask = getPatAlgosToolsTask(process)

process.load("Configuration.Geometry.GeometryRecoDB_cff")
process.load("Configuration.StandardSequences.MagneticField_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.load("Configuration.EventContent.EventContentCosmics_cff")
process.load("FWCore.MessageService.MessageLogger_cfi")

process.MessageLogger.cerr.FwkReport.reportEvery = 1000

process.options = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(True)
)

isMC = False

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(10000)
)

process.load("PhysicsTools.PatAlgos.slimming.slimmedAddPileupInfo_cfi")

process.source = cms.Source(
    "PoolSource",
    fileNames = cms.untracked.vstring(
        "/store/data/Run2026A/Muon0/MINIAOD/PromptReco-v1/000/401/624/00000/ed62979b-585c-44e1-8407-5c98f6ba1a09.root"
        #"/store/mc/RunIII2024Summer24MiniAODv6/TTtoLNu2Q_Fil-HT-500-NJet-9_Par-Hdamp-158_TuneCP5_13p6TeV_powheg-pythia8/MINIAODSIM/150X_mcRun3_2024_realistic_v2-v2/2550000/00ddac49-5df3-4afc-8c01-a5d7fb30ab07.root"
    )
)

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, "160X_dataRun3_Prompt_v1", "")
#process.GlobalTag = GlobalTag(process.GlobalTag, "150X_mcRun3_2024_realistic_v2", "")


jecLevels = ["L1FastJet", "L2Relative", "L3Absolute"]
if not isMC:
    jecLevels.append("L2L3Residual")

from PhysicsTools.PatAlgos.tools.jetTools import updateJetCollection

_btagDiscriminators = []
pnetAK8Discriminators = []

from RecoBTag.ONNXRuntime.pfParticleNetFromMiniAODAK4_cff import \
    _pfParticleNetFromMiniAODAK4PuppiCentralJetTagsAll as pfParticleNetFromMiniAODAK4PuppiCentralJetTagsAll
_btagDiscriminators += pfParticleNetFromMiniAODAK4PuppiCentralJetTagsAll

from RecoBTag.ONNXRuntime.pfParticleNetFromMiniAODAK8_cff import \
    _pfParticleNetFromMiniAODAK8JetTagsAll as pfParticleNetFromMiniAODAK8JetTagsAll
pnetAK8Discriminators += pfParticleNetFromMiniAODAK8JetTagsAll

deep_discriminators = [
    "pfMassDecorrelatedDeepBoostedDiscriminatorsJetTags:TvsQCD",
    "pfMassDecorrelatedDeepBoostedDiscriminatorsJetTags:WvsQCD",
    "pfMassDecorrelatedDeepBoostedDiscriminatorsJetTags:ZvsQCD",
    "pfMassDecorrelatedDeepBoostedDiscriminatorsJetTags:ZHbbvsQCD",
    "pfMassDecorrelatedDeepBoostedDiscriminatorsJetTags:bbvsLight",
    "pfMassDecorrelatedParticleNetJetTags:probXbb",
    "pfMassDecorrelatedParticleNetJetTags:probQCDbb",
    "pfMassDecorrelatedParticleNetJetTags:probQCDcc",
    "pfMassDecorrelatedParticleNetJetTags:probQCDb",
    "pfMassDecorrelatedParticleNetJetTags:probQCDc",
    "pfMassDecorrelatedParticleNetJetTags:probQCDothers",
    "pfMassDecorrelatedParticleNetDiscriminatorsJetTags:XbbvsQCD",
    "pfMassDecorrelatedParticleNetDiscriminatorsJetTags:XccvsQCD",
    "pfMassDecorrelatedParticleNetDiscriminatorsJetTags:XqqvsQCD",
    "pfParticleNetDiscriminatorsJetTags:TvsQCD",
    "pfParticleNetDiscriminatorsJetTags:WvsQCD",
    "pfParticleNetDiscriminatorsJetTags:ZvsQCD",
]

from RecoBTag.ONNXRuntime.pfParticleNet_cff import \
    _pfParticleNetJetTagsAll as pfParticleNetJetTagsAll
deep_discriminators += pfParticleNetJetTagsAll

updateJetCollection(
    process,
    jetSource = cms.InputTag("slimmedJetsPuppi"),
    pvSource = cms.InputTag("offlineSlimmedPrimaryVertices"),
    svSource = cms.InputTag("slimmedSecondaryVertices"),
    jetCorrections = (
        "AK4PFPuppi",
        cms.vstring(jecLevels),
        "None"
    ),
    btagDiscriminators = _btagDiscriminators,
    postfix = "WithPNetInfo",
)

ak4JetSrcForPUJetID = "selectedUpdatedPatJetsWithPNetInfo"

from RecoJets.JetProducers.PileupJetID_cfi import pileupJetId
process.pileupJetIdUpdated = pileupJetId.clone(
    jets = cms.InputTag(ak4JetSrcForPUJetID),
    inputIsCorrected = True,
    applyJec = False,
    vertexes = cms.InputTag("offlineSlimmedPrimaryVertices")
)
patAlgosToolsTask.add(process.pileupJetIdUpdated)

updateJetCollection(
    process,
    labelName = "PileupJetID",
    jetSource = cms.InputTag(ak4JetSrcForPUJetID),
)

process.updatedPatJetsPileupJetID.userData.userInts.src = [
    "pileupJetIdUpdated:fullId"
]
process.updatedPatJetsPileupJetID.userData.userFloats.src = [
    "pileupJetIdUpdated:fullDiscriminant"
]

updateJetCollection(
    process,
    jetSource = cms.InputTag("slimmedJetsAK8"),
    btagDiscriminators = pnetAK8Discriminators,
    postfix = "AK8WithPNetInfo",
    jetCorrections = (
        "AK8PFPuppi",
        cms.vstring(jecLevels),
        "None"
    ),
)

updateJetCollection(
    process,
    jetSource = cms.InputTag("selectedUpdatedPatJetsAK8WithPNetInfo"),
    btagDiscriminators = deep_discriminators,
    postfix = "AK8WithPNetInfoAll",
    jetCorrections = (
        "AK8PFPuppi",
        cms.vstring(jecLevels),
        "None"
    ),
)

from PhysicsTools.NanoTuples.ak15_cff import setupAK15
setupAK15(
    process,
    runOnMC = isMC,
    path = "p"
)
## GlobalParT
from BoostedHLT.boosted_hlt_sf.pfGlobalParticleTransformerAK15TagInfos_cfi import pfGlobalParticleTransformerAK15TagInfos
process.pfGlobalParticleTransformerAK15TagInfos = pfGlobalParticleTransformerAK15TagInfos.clone(
    min_jet_pt = 150,
    jet_radius = 1.5,
    max_jet_eta = 2.5,
    vertices = 'offlineSlimmedPrimaryVertices',
    secondary_vertices = 'slimmedSecondaryVertices',
    pf_candidates = 'packedPFCandidates',
    jets = 'selectedUpdatedPatJetsAK15ParticleNet',
    lost_tracks = 'lostTracks',
    use_puppiP4 = False,
)

from RecoBTag.ONNXRuntime.boostedJetONNXJetTagsProducer_cfi import boostedJetONNXJetTagsProducer
physics_labels = [
    'probXbb',
    'probXcc',
    'probXcs',
    'probXqq',
    'probXtauhtaue',
    'probXtauhtaum',
    'probXtauhtauh',
    'probXWW4q',
    'probXWW3q',
    'probXWWqqev',
    'probXWWqqmv',
    'probTopbWqq',
    'probTopbWq',
    'probTopbWev',
    'probTopbWmv',
    'probTopbWtauhv',
    'probQCD',
    'massCorrX2p',
    'massCorrGeneric',
    'probWithMassTopvsQCD',
    'probWithMassWvsQCD',
    'probWithMassZvsQCD',
]

process.pfGlobalParticleTransformerAK15JetTags = boostedJetONNXJetTagsProducer.clone(
    src = 'pfGlobalParticleTransformerAK15TagInfos',
    preprocess_json = 'BoostedHLT/boosted_hlt_sf/data/GloParTAK15/preprocess.json',
    model_path = 'BoostedHLT/boosted_hlt_sf/data/GloParTAK15/model_ak15.onnx',
    flav_names=physics_labels + [
        f'hidNeuron{i:03d}' for i in range(750 - len(physics_labels))
    ],
    debugMode=False,
)

pfGlobalParticleTransformerAK15JetTagsProbs = ['pfGlobalParticleTransformerAK15JetTags:' + flav_name for flav_name in process.pfGlobalParticleTransformerAK15JetTags.flav_names]

from PhysicsTools.PatAlgos.producersLayer1.jetUpdater_cfi import updatedPatJets
process.slimmedJetsAK15WithGloParT = updatedPatJets.clone(
    jetSource = "selectedUpdatedPatJetsAK15ParticleNet",
    addJetCorrFactors = False
)
process.slimmedJetsAK15WithGloParT.discriminatorSources += pfGlobalParticleTransformerAK15JetTagsProbs
patAlgosToolsTask.add(process.pfGlobalParticleTransformerAK15TagInfos)
patAlgosToolsTask.add(process.pfGlobalParticleTransformerAK15JetTags)
patAlgosToolsTask.add(process.slimmedJetsAK15WithGloParT)

process.mytuple = cms.EDAnalyzer(
    "boostedhbb",
    pileupInfo = cms.untracked.InputTag("slimmedAddPileupInfo"),
    GenJets = cms.InputTag("slimmedGenJets"),
    packedGenParticles = cms.InputTag("packedGenParticles"),
    Jets = cms.InputTag("selectedUpdatedPatJetsPileupJetID"),
    subjets = cms.untracked.string("SoftDropPuppi"),
    PFJetsAK8 = cms.InputTag("selectedUpdatedPatJetsAK8WithPNetInfoAll"),
    Electrons = cms.InputTag("slimmedElectrons"),
    Muons = cms.InputTag("slimmedMuons"),
    labe_rho = cms.InputTag("fixedGridRhoFastjetAll"),
    vertices = cms.InputTag("offlineSlimmedPrimaryVertices"),
    PFMETs = cms.InputTag("slimmedMETs"),
    PuppiMet = cms.InputTag("slimmedMETsPuppi"),
    bits = cms.InputTag("TriggerResults", "", "HLT"),
    objects = cms.InputTag("slimmedPatTrigger"),
    metfilterspatLabel_ = cms.untracked.InputTag("TriggerResults::PAT"),
    metfiltersrecoLabel_ = cms.untracked.InputTag("TriggerResults::RECO"),
    GenInf = cms.InputTag("generator", "", "SIM"),
    Vertices = cms.InputTag("offlineSlimmedPrimaryVertices"),
    l1GtSrc = cms.InputTag("gtStage2Digis"),
    genParticles = cms.InputTag("prunedGenParticles"),
    #ak15Jets = cms.InputTag("selectedUpdatedPatJetsAK15ParticleNet"),
    ak15Jets = cms.InputTag("slimmedJetsAK15WithGloParT"),
    is_MC = cms.bool(isMC)
)

process.TFileService = cms.Service(
    "TFileService",
    fileName = cms.string("output.root")
)

process.p = cms.Path(process.mytuple)
process.p.associate(patAlgosToolsTask)

process.MessageLogger.cerr.FwkReport.reportEvery = 1000
