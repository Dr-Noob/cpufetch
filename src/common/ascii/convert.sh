#!/bin/bash

cat intel.txt | hexdump -vC | head -n-1 | cut -d' ' -f2- | cut -d'|' -f1 | tr '\n' ' ' | tr -s ' ' | tr ' ' ',' | sed "s/,/,0x/g"
