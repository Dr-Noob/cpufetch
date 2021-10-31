#!/bin/bash -u

CPUID=0x00A50F00

efamily=$(((${CPUID}>>20)&0xFF))
family=$(((${CPUID}>>8)&0xF))
emodel=$(((${CPUID}>>16)&0xF))
model=$(((${CPUID}>>4)&0xF))
stepping=$((${CPUID}&0xF))

printf 'CPUID: 0x%.8X\n' $CPUID
printf -- '- EF = 0x%X (%d)\n' $efamily $efamily
printf -- '- F  = 0x%X (%d)\n' $family $family
printf -- '- EM = 0x%X (%d)\n' $emodel $emodel
printf -- '- M  = 0x%X (%d)\n' $model $model
printf -- '- S  = 0x%X (%d)\n' $stepping $stepping

#EF=$efamily
#F=$family
#EM=$emodel
#M=$model
#S=$stepping
#grep -E "\s*CHECK_UARCH\(arch,\s*${EF},\s*${F},\s*(${EM}|NA),\s*(${M}|NA),\s*(${S}|NA)" uarch.c
