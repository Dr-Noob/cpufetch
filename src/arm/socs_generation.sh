#!/bin/bash -u

SOC_LIST="$(grep SOC_EQ soc.c | grep -v '//SOC_EQ' | grep -v 'define' | cut -d',' -f2 | sed 's/"//')"

IFS=$'"'

for soc in $SOC_LIST
do
  # CLEAN
  soc=$(echo $soc | tr -d '\n')
  soc="${soc:1}"
  
  # REPLACE
  soc=$(echo $soc | sed "s/BCM/BCM_/g")
  soc=$(echo $soc | sed "s/universal/EXYNOS_/g")
  soc=$(echo $soc | sed "s/Hi/HISILICON_/g")
  soc=$(echo $soc | sed "s/^MSM/SNAPD_MSM/g" | sed "s/SDM/SNAPD_SDM/g" | sed "s/APQ/SNAPD_APQ/g" | sed "s/^SM/SNAPD_SM/g" | sed "s/QM/SNAPD_QM/g" | sed "s/QSD/SNAPD_QSD/g")
  soc=$(echo $soc | sed "s/MT/MTK_MT/g")
  soc=$(echo $soc | sed "s/-/_/g" | sed "s/ /_/g")
  echo '  SOC_'"$soc"','
done

unset IFS
