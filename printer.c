#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "printer.h"
#include "ascii.h"
#include "global.h"

#define COL_INTEL_DEFAULT_1 "\x1b[36;1m"
#define COL_INTEL_DEFAULT_2 "\x1b[37;1m"
#define COL_INTEL_DARK_1 "\x1b[34;1m"
#define COL_INTEL_DARK_2 "\x1b[30m"
#define COL_AMD_DEFAULT_1 "\x1b[37;1m"
#define COL_AMD_DEFAULT_2 "\x1b[31;1m"
#define COL_AMD_DARK_1 "\x1b[30;1m"
#define COL_AMD_DARK_2 "\x1b[32;1m"
#define RESET "\x1b[0m"

#define TITLE_NAME      "Name:       "
#define TITLE_FREQUENCY "Frequency:  "
#define TITLE_NCORES    "N.Cores:    "
#define TITLE_AVX       "AVX:        "
#define TITLE_SSE       "SSE:        "
#define TITLE_FMA       "FMA:        "
#define TITLE_AES       "AES:        "
#define TITLE_SHA       "SHA:        "
#define TITLE_L1        "L1 Size:    "
#define TITLE_L2        "L2 Size:    "
#define TITLE_L3        "L3 Size:    "
#define TITLE_PEAK      "Peak FLOPS: "

/*** CENTER TEXT ***/
#define LINES_SPACE_UP   4
#define LINES_SPACE_DOWN 4

static const char* ATTRIBUTE_FIELDS [ATTRIBUTE_COUNT] =  {  TITLE_NAME, TITLE_FREQUENCY,
                                                            TITLE_NCORES, TITLE_AVX, TITLE_SSE,
                                                            TITLE_FMA, TITLE_AES, TITLE_SHA,
                                                            TITLE_L1, TITLE_L2, TITLE_L3,
                                                            TITLE_PEAK };

static const int ATTRIBUTE_LIST[ATTRIBUTE_COUNT] =  { ATTRIBUTE_NAME, ATTRIBUTE_FREQUENCY,
                                                        ATTRIBUTE_NCORES, ATTRIBUTE_AVX, ATTRIBUTE_SSE,
                                                        ATTRIBUTE_FMA, ATTRIBUTE_AES, ATTRIBUTE_SHA,
                                                        ATTRIBUTE_L1, ATTRIBUTE_L2, ATTRIBUTE_L3,
                                                        ATTRIBUTE_PEAK };

struct ascii {
  char art[NUMBER_OF_LINES][LINE_SIZE];
  char color1[10];
  char color2[10];
  char* atributes[ATTRIBUTE_COUNT];
  VENDOR vendor;
};

int setAttribute(struct ascii* art, int type, char* value) {
  int i = 0;
  while(i < ATTRIBUTE_COUNT && type != ATTRIBUTE_LIST[i])
    i++;
  if(i == ATTRIBUTE_COUNT)
    return BOOLEAN_FALSE;
  art->atributes[i] = value;
  return BOOLEAN_TRUE;
}

struct ascii* set_ascii(VENDOR cpuVendor, STYLE style) {
  /*** Check that number of lines of ascii art matches the number
  of spaces plus the number of lines filled with text ***/
  if(LINES_SPACE_UP+LINES_SPACE_DOWN+ATTRIBUTE_COUNT != NUMBER_OF_LINES) {
    printError("Number of lines do not match (%d vs %d)",LINES_SPACE_UP+LINES_SPACE_DOWN+ATTRIBUTE_COUNT,NUMBER_OF_LINES);
    return NULL;
  }

  struct ascii* art = malloc(sizeof(struct ascii));
  art->vendor = cpuVendor;
  if(cpuVendor == VENDOR_INTEL) {
    /*** CHECK STYLE ***/
    switch (style) {
      case STYLE_EMPTY:
      case STYLE_DEFAULT:
        strcpy(art->color1,COL_INTEL_DEFAULT_1);
        strcpy(art->color2,COL_INTEL_DEFAULT_2);
        break;
      case STYLE_DARK:
        strcpy(art->color1,COL_INTEL_DARK_1);
        strcpy(art->color2,COL_INTEL_DARK_2);
        break;
      default:
        printError("Found invalid style (%d)",style);
        return NULL;
    }

    /*** COPY ASCII-ART ***/
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
    /*** CHECK STYLE ***/
    switch (style) {
      case STYLE_EMPTY:
      case STYLE_DEFAULT:
        strcpy(art->color1,COL_AMD_DEFAULT_1);
        strcpy(art->color2,COL_AMD_DEFAULT_2);
        break;
      case STYLE_DARK:
        strcpy(art->color1,COL_AMD_DARK_1);
        strcpy(art->color2,COL_AMD_DARK_2);
        break;
      default:
        printError("Found invalid style (%d)",style);
        return NULL;
    }

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

void print_ascii_intel(struct ascii* art) {
  int flag = BOOLEAN_FALSE;

  for(int n=0;n<NUMBER_OF_LINES;n++) {

    /*** PRINT ASCII-ART ***/
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

    /*** PRINT ATTRIBUTE ***/
    if(n>LINES_SPACE_UP-1 && n<NUMBER_OF_LINES-LINES_SPACE_DOWN)printf("%s%s%s%s"RESET"\n",art->color1,ATTRIBUTE_FIELDS[n-LINES_SPACE_UP],art->color2,art->atributes[n-LINES_SPACE_UP]);
    else printf("\n");
  }
}

void print_ascii_amd(struct ascii* art) {
  int flag = BOOLEAN_FALSE;

  for(int n=0;n<NUMBER_OF_LINES;n++) {
    /*** PRINT ASCII-ART ***/
    for(int i=0;i<LINE_SIZE;i++) {
      if(art->art[n][i] == '@')
        printf("%s%c" RESET,art->color1,art->art[n][i]);
      else if(art->art[n][i] == '#')
        printf("%s%c" RESET,art->color2,art->art[n][i]);
      else
        printf("%c",art->art[n][i]);
    }

    /*** PRINT ATTRIBUTE ***/
    if(n>2 && n<NUMBER_OF_LINES-4)printf("%s%s%s%s"RESET"\n",art->color1,ATTRIBUTE_FIELDS[n-3],art->color2,art->atributes[n-3]);
    else printf("\n");
  }

}

void print_ascii(struct ascii* art) {
  if(art->vendor == VENDOR_INTEL)
    print_ascii_intel(art);
  else
    print_ascii_amd(art);
}
