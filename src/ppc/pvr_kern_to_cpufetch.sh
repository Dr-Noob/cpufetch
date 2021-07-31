#!/bin/bash

# This script takes as input cputable.c from linux kernel
# and generates a valid output for cpufetch in src/ppc/uarch.c

CPUTABLE_PATH="linux-5.13.7/arch/powerpc/kernel/cputable.c"

raw_values=$(grep '\.pvr_value' "$CPUTABLE_PATH" | grep -oP "= .*," | cut -d' ' -f2 | tr -d ',')
raw_masks=$(grep '\.pvr_mask' "$CPUTABLE_PATH" | grep -oE "0x........")

raw_v_len=$(echo "$raw_values" | wc -l)
raw_m_len=$(echo "$raw_masks" | wc -l)

if [ $raw_v_len -ne $raw_m_len ]
then
  echo "Lengths do not match!"
  echo "values length: $raw_v_len"
  echo "masks length:  $raw_m_len"
  exit 1
fi

IFS=$'\n' read -r -d ' ' -a values <<< "$raw_values"
IFS=$'\n' read -r -d ' ' -a masks <<< "$raw_masks"

for i in "${!values[@]}"
do
  echo '  CHECK_UARCH(arch, pvr, '"${masks[i]}"', '"${values[i]}"', "POWERX", UARCH_POWERX, -1)'
done
