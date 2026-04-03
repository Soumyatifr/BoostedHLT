#!/bin/bash

config=run_modules_semilepton_2026_ak15.py
publish=False
site=T2_CH_CERN
DBS=global

sample_names=(
Muon0_2026A
Muon0_2026B
Muon1_2026A
Muon1_2026B
Muon2_2026A
Muon2_2026B
Muon3_2026A
Muon3_2026B
)

sample_data=(
/Muon0/Run2026A-PromptReco-v1/MINIAOD
/Muon0/Run2026B-PromptReco-v1/MINIAOD
/Muon1/Run2026A-PromptReco-v1/MINIAOD
/Muon1/Run2026B-PromptReco-v1/MINIAOD
/Muon2/Run2026A-PromptReco-v1/MINIAOD
/Muon2/Run2026B-PromptReco-v1/MINIAOD
/Muon3/Run2026A-PromptReco-v1/MINIAOD
/Muon3/Run2026B-PromptReco-v1/MINIAOD
)

nsamples=${#sample_data[*]}
if [ $nsamples != ${#sample_names[*]} ];
then
        echo "No of names & samples are not same!! please check! (samples $nsamples names ${#sample_names[*]}"
        exit
fi

fil_list=crab_submit
mon_list=crab_monitor
truncate -s 0 ${fil_list}.sh
echo "#!/bin/bash" | cat >>${fil_list}.sh
truncate -s 0 ${mon_list}.sh

i=1
while [[ $i -le $nsamples ]]
do
        echo ${sample_data[i-1]} ${sample_names[i-1]}
        label=data_2026_${sample_names[i-1]}
        ./crab_write_data_2026.sh $label $config  ${sample_data[i-1]} $publish $site $DBS
        echo "crab submit -c crabfile_${label}.py" | cat >>${fil_list}.sh
        echo "crab status -d crab_${label}/crab_crab_${label}/" | cat >>${mon_list}.sh
        ((i = i + 1))
done

chmod 744 ${fil_list}.sh
chmod 744 ${mon_list}.sh
