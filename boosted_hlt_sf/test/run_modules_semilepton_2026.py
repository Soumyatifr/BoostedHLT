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
    allowUnscheduled=cms.untracked.bool(True),
    wantSummary=cms.untracked.bool(True)
)

isMC = False

process.maxEvents = cms.untracked.PSet(
    input=cms.untracked.int32(-1)
)
# process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(1))

process.load("PhysicsTools.PatAlgos.slimming.slimmedAddPileupInfo_cfi")

process.source = cms.Source(
    "PoolSource",
    fileNames=cms.untracked.vstring(
        "/store/data/Run2026A/Muon0/MINIAOD/PromptReco-v1/000/401/624/00000/ed62979b-585c-44e1-8407-5c98f6ba1a09.root"
    )
)

# ----------------------------------------------------------------------
# GlobalTag
# ----------------------------------------------------------------------
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, "160X_dataRun3_Prompt_v1", "")

# ----------------------------------------------------------------------
# Default JEC levels from GlobalTag
# ----------------------------------------------------------------------
jecLevels = ["L1FastJet", "L2Relative", "L3Absolute"]
if not isMC:
    jecLevels.append("L2L3Residual")

# ----------------------------------------------------------------------
# ParticleNet discriminators
# ----------------------------------------------------------------------
from PhysicsTools.PatAlgos.tools.jetTools import updateJetCollection

_btagDiscriminators = []
pnetAK8Discriminators = []

from RecoBTag.ONNXRuntime.pfParticleNetFromMiniAODAK4_cff import \
    _pfParticleNetFromMiniAODAK4PuppiCentralJetTagsAll as pfParticleNetFromMiniAODAK4PuppiCentralJetTagsAll
_btagDiscriminators += pfParticleNetFromMiniAODAK4PuppiCentralJetTagsAll

from RecoBTag.ONNXRuntime.pfParticleNetFromMiniAODAK8_cff import \
    _pfParticleNetFromMiniAODAK8JetTagsAll as pfParticleNetFromMiniAODAK8JetTagsAll
pnetAK8Discriminators += pfParticleNetFromMiniAODAK8JetTagsAll

print(_btagDiscriminators)
print(pnetAK8Discriminators)

# ----------------------------------------------------------------------
# Additional AK8 discriminators
# ----------------------------------------------------------------------
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

print(deep_discriminators)

# ----------------------------------------------------------------------
# AK4 PUPPI jets with default JEC + PNet
# ----------------------------------------------------------------------
updateJetCollection(
    process,
    jetSource=cms.InputTag("slimmedJetsPuppi"),
    pvSource=cms.InputTag("offlineSlimmedPrimaryVertices"),
    svSource=cms.InputTag("slimmedSecondaryVertices"),
    jetCorrections=(
        "AK4PFPuppi",
        cms.vstring(jecLevels),
        "None"
    ),
    btagDiscriminators=_btagDiscriminators,
    postfix="WithPNetInfo",
)

# ----------------------------------------------------------------------
# For data, do not smear jets
# ----------------------------------------------------------------------
ak4JetSrcForPUJetID = "selectedUpdatedPatJetsWithPNetInfo"

# ----------------------------------------------------------------------
# Pileup Jet ID
# ----------------------------------------------------------------------
from RecoJets.JetProducers.PileupJetID_cfi import pileupJetId
process.pileupJetIdUpdated = pileupJetId.clone(
    jets=cms.InputTag(ak4JetSrcForPUJetID),
    inputIsCorrected=True,
    applyJec=False,
    vertexes=cms.InputTag("offlineSlimmedPrimaryVertices")
)
patAlgosToolsTask.add(process.pileupJetIdUpdated)

updateJetCollection(
    process,
    labelName="PileupJetID",
    jetSource=cms.InputTag(ak4JetSrcForPUJetID),
)

process.updatedPatJetsPileupJetID.userData.userInts.src = [
    "pileupJetIdUpdated:fullId"
]
process.updatedPatJetsPileupJetID.userData.userFloats.src = [
    "pileupJetIdUpdated:fullDiscriminant"
]

# ----------------------------------------------------------------------
# AK8 PUPPI jets with default JEC + PNet
# ----------------------------------------------------------------------
updateJetCollection(
    process,
    jetSource=cms.InputTag("slimmedJetsAK8"),
    btagDiscriminators=pnetAK8Discriminators,
    postfix="AK8WithPNetInfo",
    jetCorrections=(
        "AK8PFPuppi",
        cms.vstring(jecLevels),
        "None"
    ),
)

updateJetCollection(
    process,
    jetSource=cms.InputTag("selectedUpdatedPatJetsAK8WithPNetInfo"),
    btagDiscriminators=deep_discriminators,
    postfix="AK8WithPNetInfoAll",
    jetCorrections=(
        "AK8PFPuppi",
        cms.vstring(jecLevels),
        "None"
    ),
)

# ----------------------------------------------------------------------
# Analyzer
# GParT removed completely
# PFJetsAK8 now points to selectedUpdatedPatJetsAK8WithPNetInfoAll
# ----------------------------------------------------------------------
process.mytuple = cms.EDAnalyzer(
    "boostedhbb",
    pileupInfo=cms.untracked.InputTag("slimmedAddPileupInfo"),
    GenJets=cms.InputTag("slimmedGenJets"),
    packedGenParticles=cms.InputTag("packedGenParticles"),
    Jets=cms.InputTag("selectedUpdatedPatJetsPileupJetID"),
    subjets=cms.untracked.string("SoftDropPuppi"),
    PFJetsAK8=cms.InputTag("selectedUpdatedPatJetsAK8WithPNetInfoAll"),
    Electrons=cms.InputTag("slimmedElectrons"),
    Muons=cms.InputTag("slimmedMuons"),
    labe_rho=cms.InputTag("fixedGridRhoFastjetAll"),
    vertices=cms.InputTag("offlineSlimmedPrimaryVertices"),
    PFMETs=cms.InputTag("slimmedMETs"),
    PuppiMet=cms.InputTag("slimmedMETsPuppi"),
    bits=cms.InputTag("TriggerResults", "", "HLT"),
    objects=cms.InputTag("slimmedPatTrigger"),
    metfilterspatLabel_=cms.untracked.InputTag("TriggerResults::PAT"),
    metfiltersrecoLabel_=cms.untracked.InputTag("TriggerResults::RECO"),
    GenInf=cms.InputTag("generator", "", "SIM"),
    Vertices=cms.InputTag("offlineSlimmedPrimaryVertices"),
    l1GtSrc=cms.InputTag("gtStage2Digis"),
    genParticles=cms.InputTag("prunedGenParticles"),
    is_MC=cms.bool(isMC)
)

process.TFileService = cms.Service(
    "TFileService",
    fileName=cms.string("output.root")
)

process.p = cms.Path(
    process.mytuple,
    patAlgosToolsTask
)

process.MessageLogger.cerr.FwkReport.reportEvery = 1000
