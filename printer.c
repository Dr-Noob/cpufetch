#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "printer.h"
#include "ascii.h"

#define COL_INTEL_1 "\x1b[34;1m"
#define COL_INTEL_2 "\x1b[37;1m"
#define COL_AMD_1 "\x1b[30;1m"
#define COL_AMD_2 "\x1b[32;1m"
#define RESET "\x1b[0m"

struct ascii {
  char art[NUMBER_OF_LINES][LINE_SIZE];
  char color1[10];
  char color2[10];
  VENDOR vendor;
};

struct ascii* set_ascii(VENDOR cpuVendor) {
  struct ascii* art = malloc(sizeof(struct ascii));
  art->vendor = cpuVendor;
  if(cpuVendor == VENDOR_INTEL) {
    strcpy(art->color1,COL_INTEL_1);
    strcpy(art->color2,COL_INTEL_2);

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
    strcpy(art->color1,COL_AMD_1);
    strcpy(art->color2,COL_AMD_2);

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

void print_text(struct ascii* art,char* title, char* text) {
  printf("%s%s%s%s"RESET"\n",art->color1,title,art->color2,text);
}

void print_ascii(struct ascii* art, int n) {
  int flag = BOOLEAN_FALSE;

  if(art->vendor == VENDOR_INTEL) {
    /*** PRINT ASCII WITH SHADOW ***/
    for(int i=0;i<LINE_SIZE;i++) {
      if(flag) {
        if(art->art[n][i] == ' ') {
          flag = BOOLEAN_FALSE;
          printf("%c",art->art[n][i]);
        }
        else
          printf("%s%c" RESET,art->color1,art->art[n][i]);
      }
      else {
        if(art->art[n][i] != ' ') {
          flag = BOOLEAN_TRUE;
          printf("%s%c" RESET,art->color2,art->art[n][i]);
        }
        else
          printf("%c",art->art[n][i]);
      }
    }
  }
  else {
    /*** PRINT TEXT AND LOGO IN DIFFERENT COLOR ***/
    for(int i=0;i<LINE_SIZE;i++) {
      if(art->art[n][i] == '@')
        printf("%s%c" RESET,art->color1,art->art[n][i]);
      else if(art->art[n][i] == '#')
        printf("%s%c" RESET,art->color2,art->art[n][i]);
      else
        printf("%c",art->art[n][i]);
    }
  }

}

/*** PRINT ASCII SIMPLE ***/
/*
void print_ascii(struct ascii* art, int n) {
  int flag = BOOLEAN_FALSE;

  for(int i=0;i<LINE_SIZE;i++) {
    if(art->art[n][i] != ' ')
      printf(BLUE "%c" RESET,art->art[n][i]);
    else
      printf("%c",art->art[n][i]);
  }

}
*/
