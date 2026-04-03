# BoostedHLT

## Overview

**BoostedHLT** is a CMSSW EDAnalyzer package developed to study boosted-Higgs trigger performance and ParticleNet-based tagging at the High-Level Trigger (HLT). The package provides tools to:

- evaluate HLT ParticleNet tagging behaviour
- compare online vs offline ParticleNet discriminators
- compute trigger efficiencies
- derive scale factors
- produce ntuples for Run-3 boosted-Hbb analyses

This package is designed for MiniAOD-based workflows and supports both **data** and **MC** processing.

test/

Contains runtime configuration scripts.

These scripts are the main entry points for executing the analyzer.


Important configuration files:

run_modules_semilepton_2026.py

Standard Run-3 MiniAOD workflow configuration.

run_modules_semilepton_2026_ak15.py 

To test on the tt simulated sample for the cross check purpose

--------------------------------------------------
Installation
--------------------------------------------------

Inside a CMSSW release area:

cmsrel CMSSW_16_0_X
cd CMSSW_16_0_X/src
cmsenv


