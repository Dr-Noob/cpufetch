#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "printer.h"
#include "ascii.h"

struct ascii {
  char art[NUMBER_OF_LINES][LINE_SIZE];
};

struct ascii* set_ascii(VENDOR cpuVendor) {
  struct ascii* art = malloc(sizeof(struct ascii));
  if(cpuVendor == VENDOR_INTEL) {
    strcpy(art->art[0],INTEL1);
    strcpy(art->art[1],INTEL2);
    strcpy(art->art[2],INTEL3);
    strcpy(art->art[3],INTEL4);
    strcpy(art->art[4],INTEL5);
    strcpy(art->art[5],INTEL6);
    strcpy(art->art[6],INTEL7);
    strcpy(art->art[7],INTEL8);
    strcpy(art->art[8],INTEL9);
    strcpy(art->art[9],INTEL10);
    strcpy(art->art[10],INTEL11);
    strcpy(art->art[11],INTEL12);
    strcpy(art->art[12],INTEL13);
    strcpy(art->art[13],INTEL14);
    strcpy(art->art[14],INTEL15);
    strcpy(art->art[15],INTEL16);
    strcpy(art->art[16],INTEL17);
    strcpy(art->art[17],INTEL18);
    strcpy(art->art[18],INTEL19);
    strcpy(art->art[19],INTEL20);
  }
  else {
    strcpy(art->art[0],AMD1);
    strcpy(art->art[1],AMD2);
    strcpy(art->art[2],AMD3);
    strcpy(art->art[3],AMD4);
    strcpy(art->art[4],AMD5);
    strcpy(art->art[5],AMD6);
    strcpy(art->art[6],AMD7);
    strcpy(art->art[7],AMD8);
    strcpy(art->art[8],AMD9);
    strcpy(art->art[9],AMD10);
    strcpy(art->art[10],AMD11);
    strcpy(art->art[11],AMD12);
    strcpy(art->art[12],AMD13);
    strcpy(art->art[13],AMD14);
    strcpy(art->art[14],AMD15);
    strcpy(art->art[15],AMD16);
    strcpy(art->art[16],AMD17);
    strcpy(art->art[17],AMD18);
    strcpy(art->art[18],AMD19);
    strcpy(art->art[19],AMD20);
  }
  return art;
}

void print_ascii(struct ascii* art, int n) {
  printf("%s",art->art[n]);
}
