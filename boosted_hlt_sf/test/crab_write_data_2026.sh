#!/bin/sh
production_tag=$1
config=$2
Dataset=$3
publication=$4
site=$5
DBS=$6

temp=crabfile_${1}.py

truncate -s 0 $temp

echo "import os
from CRABClient.UserUtilities import config
config = config()

config.General.requestName = 'crab_${production_tag}'
config.General.workArea = 'crabworkarea'
config.General.transferOutputs = True
config.General.transferLogs = False
config.JobType.maxJobRuntimeMin = 2700
config.JobType.maxMemoryMB = 5000
config.JobType.numCores = 2
config.JobType.pluginName  = 'Analysis'
config.JobType.psetName    = '${config}'
config.JobType.outputFiles = ['output.root']
config.Data.inputDataset = '$Dataset'
config.Data.inputDBS = '$DBS'
config.Data.splitting = 'FileBased'

config.Data.unitsPerJob = 1
config.Data.outLFNDirBase = '/store/group/phys_higgs/mukherje/UCSD_HBB/Run-3/Boosted_HLT_2026/'
config.Site.storageSite = '$site'

config.section_(\"Debug\")
config.Debug.extraJDL = ['+CMS_ALLOW_OVERFLOW=False']
#config.Site.whitelist = [\"T2_IN_TIFR\"]" | cat >>$temp
