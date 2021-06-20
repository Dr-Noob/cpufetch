#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "soc.h"
#include "socs.h"
#include "udev.h"
#include "../common/global.h"

#define min(a,b) (((a)<(b))?(a):(b))
#define STRING_UNKNOWN    "Unknown"
#define ARRAY_SIZE(arr)     (sizeof(arr) / sizeof((arr)[0]))

static char* soc_trademark_string[] = {
  [SOC_VENDOR_SNAPDRAGON] = "Snapdragon ",
  [SOC_VENDOR_MEDIATEK]   = "MediaTek ",
  [SOC_VENDOR_EXYNOS]     = "Exynos ",
  [SOC_VENDOR_KIRIN]      = "Kirin ",
  [SOC_VENDOR_BROADCOM]   = "Broadcom BCM",
};

static char* soc_rpi_string[] = {
  "BCM2835",
  "BCM2836",
  "BCM2837",
  "BCM2711"
};

void fill_soc(struct system_on_chip* soc, char* soc_name, SOC soc_model, int32_t process) {
  soc->soc_model = soc_model;
  soc->soc_vendor = get_soc_vendor_from_soc(soc_model);
  soc->process = process;
  int len = strlen(soc_name) + strlen(soc_trademark_string[soc->soc_vendor]) + 1;
  soc->soc_name = malloc(sizeof(char) * len);
  memset(soc->soc_name, 0, sizeof(char) * len);
  sprintf(soc->soc_name, "%s%s", soc_trademark_string[soc->soc_vendor], soc_name);    
}

bool match_soc(struct system_on_chip* soc, char* raw_name, char* expected_name, char* soc_name, SOC soc_model, int32_t process) {
  if(strlen(raw_name) > strlen(expected_name))
    return false;
  
  int len = strlen(raw_name);
  if(strncmp(raw_name, expected_name, len) != 0) {
    return false;
  }
  else {
    fill_soc(soc, soc_name, soc_model, process);
    return true;    
  }
}

char* toupperstr(char* str) {
  int len = strlen(str) + 1;
  char* ret = malloc(sizeof(char) * len);
  memset(ret, 0, sizeof(char) * len);
  
  for(int i=0; i < len; i++) {
    ret[i] = toupper((unsigned char) str[i]);    
  }
  
  return ret;
}

#define SOC_START if (false) {}
#define SOC_EQ(raw_name, expected_name, soc_name, soc_model, soc, process) \
   else if (match_soc(soc, raw_name, expected_name, soc_name, soc_model, process)) return true;
#define SOC_END else { return false; }

// https://en.wikipedia.org/wiki/Raspberry_Pi
// http://phonedb.net/index.php?m=processor&id=562&c=broadcom_bcm21663
// https://hwbot.org/hardware/processors#key=bcmxxx
bool match_broadcom(char* soc_name, struct system_on_chip* soc) {
  char* tmp;

  if((tmp = strstr(soc_name, "BCM")) == NULL)
    return false;
  
  SOC_START
  SOC_EQ(tmp, "BCM2835",              "2835",              SOC_BCM_2835,   soc, 65)    
  SOC_EQ(tmp, "BCM2836",              "2836",              SOC_BCM_2836,   soc, 40)    
  SOC_EQ(tmp, "BCM2837",              "2837",              SOC_BCM_2837,   soc, 40) 
  SOC_EQ(tmp, "BCM2837B0",            "2837B0",            SOC_BCM_2837B0, soc, 40) 
  SOC_EQ(tmp, "BCM2711",              "2711",              SOC_BCM_2711,   soc, 28)  
  SOC_EQ(tmp, "BCM21553",             "21553",             SOC_BCM_21553,  soc, 65)    
  SOC_EQ(tmp, "BCM21553-Thunderbird", "21553 Thunderbird", SOC_BCM_21553T, soc, 65)      
  SOC_EQ(tmp, "BCM21663",             "21663",             SOC_BCM_21663,  soc, 40) 
  SOC_EQ(tmp, "BCM21664",             "21664",             SOC_BCM_21664,  soc, 40) 
  SOC_EQ(tmp, "BCM28155",             "28155",             SOC_BCM_28155,  soc, 40) 
  SOC_EQ(tmp, "BCM23550",             "23550",             SOC_BCM_23550,  soc, 40) 
  SOC_EQ(tmp, "BCM28145",             "28145",             SOC_BCM_28145,  soc, 40) 
  SOC_EQ(tmp, "BCM2157",              "2157",              SOC_BCM_2157,   soc, 65) 
  SOC_EQ(tmp, "BCM21654",             "21654",             SOC_BCM_21654,  soc, 40) 
  SOC_END
}

// https://www.techinsights.com/
// https://datasheetspdf.com/pdf-file/1316605/HiSilicon/Hi3660/1
bool match_hisilicon(char* soc_name, struct system_on_chip* soc) {
  char* tmp;

  if((tmp = strstr(soc_name, "Hi")) == NULL)
    return false;
  
  SOC_START
  SOC_EQ(tmp, "Hi3620GFC",  "K3V2",  SOC_HISILICON_3620, soc, 40)
  //SOC_EQ(tmp, "?",        "K3V2E", SOC_KIRIN, soc,  ?)
  //SOC_EQ(tmp, "?",        "620",   SOC_KIRIN, soc, 28)
  //SOC_EQ(tmp, "?",        "650",   SOC_KIRIN, soc, 16)
  //SOC_EQ(tmp, "?",        "655",   SOC_KIRIN, soc, 16)
  //SOC_EQ(tmp, "?",        "658",   SOC_KIRIN, soc, 16)
  //SOC_EQ(tmp, "?",        "659",   SOC_KIRIN, soc, 16)
  //SOC_EQ(tmp, "?",        "710",   SOC_KIRIN, soc, 12)
  //SOC_EQ(tmp, "?",        "710A",  SOC_KIRIN, soc, 12)
  //SOC_EQ(tmp, "?",        "710F",  SOC_KIRIN, soc, 12)
  //SOC_EQ(tmp, "?",        "810",   SOC_KIRIN, soc,  7)
  //SOC_EQ(tmp, "?",        "820",   SOC_KIRIN, soc,  7)
  //SOC_EQ(tmp, "?",        "9000",  SOC_KIRIN, soc,  5)
  //SOC_EQ(tmp, "?",        "9000E", SOC_KIRIN, soc,  5)
  //SOC_EQ(tmp, "?",        "910",   SOC_KIRIN, soc, 28)
  //SOC_EQ(tmp, "?",        "910T",  SOC_KIRIN, soc, 28)
  SOC_EQ(tmp, "Hi3630",     "920",   SOC_HISILICON_3630, soc, 28)
  //SOC_EQ(tmp, "?",        "925",   SOC_KIRIN, soc, 28)
  //SOC_EQ(tmp, "?",        "930",   SOC_KIRIN, soc, ?)
  //SOC_EQ(tmp, "?",        "935",   SOC_KIRIN, soc, ?)
  SOC_EQ(tmp, "Hi3650",     "950",   SOC_HISILICON_3650, soc, 16)
  //SOC_EQ(tmp, "?",        "955",   SOC_KIRIN, soc, ?)
  SOC_EQ(tmp, "Hi3660",     "960",   SOC_HISILICON_3660, soc, 16)
  //SOC_EQ(tmp, "?",        "960S",  SOC_KIRIN, soc, 16)
  SOC_EQ(tmp, "Hi3670",     "970",   SOC_HISILICON_3670, soc, 10)
  SOC_EQ(tmp, "Hi3680",     "980",   SOC_HISILICON_3680, soc,  7)
  //SOC_EQ(tmp, "?",        "985",   SOC_KIRIN, soc,  7)
  SOC_EQ(tmp, "Hi3690",     "990",   SOC_HISILICON_3690, soc,  7)
  SOC_END
}

bool match_exynos(char* soc_name, struct system_on_chip* soc) {
  char* tmp;

  if((tmp = strstr(soc_name, "universal")) == NULL)
    return false;
  
  SOC_START
  // universalXXXX //
  SOC_EQ(tmp, "universal3475", "3475", SOC_EXYNOS_3475, soc, 28) 
  SOC_EQ(tmp, "universal4210", "4210", SOC_EXYNOS_4210, soc, 45)
  SOC_EQ(tmp, "universal4212", "4212", SOC_EXYNOS_4212, soc, 32)
  SOC_EQ(tmp, "universal4412", "4412", SOC_EXYNOS_4412, soc, 32)
  SOC_EQ(tmp, "universal5250", "5250", SOC_EXYNOS_5250, soc, 32)
  SOC_EQ(tmp, "universal5410", "5410", SOC_EXYNOS_5410, soc, 28)
  SOC_EQ(tmp, "universal5420", "5420", SOC_EXYNOS_5420, soc, 28)
  SOC_EQ(tmp, "universal5422", "5422", SOC_EXYNOS_5422, soc, 28)
  SOC_EQ(tmp, "universal5430", "5430", SOC_EXYNOS_5430, soc, 20)
  SOC_EQ(tmp, "universal5433", "5433", SOC_EXYNOS_5433, soc, 20)
  SOC_EQ(tmp, "universal5260", "5260", SOC_EXYNOS_5260, soc, 28)
  SOC_EQ(tmp, "universal7270", "7270", SOC_EXYNOS_7270, soc, 14)
  SOC_EQ(tmp, "universal7420", "7420", SOC_EXYNOS_7420, soc, 14)
  SOC_EQ(tmp, "universal7570", "7570", SOC_EXYNOS_7570, soc, 14)
  SOC_EQ(tmp, "universal7870", "7870", SOC_EXYNOS_7870, soc, 14)
  SOC_EQ(tmp, "universal7872", "7872", SOC_EXYNOS_7872, soc, 14)
  SOC_EQ(tmp, "universal7880", "7880", SOC_EXYNOS_7880, soc, 14)
  SOC_EQ(tmp, "universal7884", "7884", SOC_EXYNOS_7884, soc, 14)
  SOC_EQ(tmp, "universal7885", "7885", SOC_EXYNOS_7885, soc, 14)
  SOC_EQ(tmp, "universal7904", "7904", SOC_EXYNOS_7904, soc, 14)
  SOC_EQ(tmp, "universal8890", "8890", SOC_EXYNOS_8890, soc, 14)
  SOC_EQ(tmp, "universal8895", "8895", SOC_EXYNOS_8895, soc, 10)
  SOC_EQ(tmp, "universal9110", "9110", SOC_EXYNOS_9110, soc, 14)
  SOC_EQ(tmp, "universal9609", "9609", SOC_EXYNOS_9609, soc, 10)
  SOC_EQ(tmp, "universal9610", "9610", SOC_EXYNOS_9610, soc, 10)
  SOC_EQ(tmp, "universal9611", "9611", SOC_EXYNOS_9611, soc, 10)
  SOC_EQ(tmp, "universal9810", "9810", SOC_EXYNOS_9810, soc, 10)
  SOC_EQ(tmp, "universal9820", "9820", SOC_EXYNOS_9820, soc,  8)
  SOC_EQ(tmp, "universal9825", "9825", SOC_EXYNOS_9825, soc,  7)
  // New exynos. Dont know if they will work //
  SOC_EQ(tmp, "universal1080", "1080", SOC_EXYNOS_1080, soc,  5)  
  SOC_EQ(tmp, "universal990",   "990", SOC_EXYNOS_990,  soc,  7)
  SOC_EQ(tmp, "universal980",   "980", SOC_EXYNOS_980,  soc,  8)    
  SOC_EQ(tmp, "universal880",   "880", SOC_EXYNOS_880,  soc,  8)
  SOC_END
}

bool match_mediatek(char* soc_name, struct system_on_chip* soc) {
  char* tmp;

  if((tmp = strstr(soc_name, "MT")) == NULL)
    return false;
  
  SOC_START
  // Dimensity //
  SOC_EQ(tmp, "MT6889",   "Dimensity 1000",  SOC_MTK_MT6889,   soc, 7) 
  SOC_EQ(tmp, "MT6885Z",  "Dimensity 1000L", SOC_MTK_MT6885Z,  soc, 7) 
  //SOC_EQ(tmp, "?",      "Dimensity 700",   SOC_MTK_,         soc, 7) 
  SOC_EQ(tmp, "MT6853",   "Dimensity 720",   SOC_MTK_MT6853,   soc, 7) 
  SOC_EQ(tmp, "MT6873",   "Dimensity 800",   SOC_MTK_MT6873,   soc, 7)
  SOC_EQ(tmp, "MT6875",   "Dimensity 820",   SOC_MTK_MT6875,   soc, 7)
  // Helio //  
  SOC_EQ(tmp, "MT6761D",  "Helio A20",       SOC_MTK_MT6761D,  soc, 12)
  SOC_EQ(tmp, "MT6761",   "Helio A22",       SOC_MTK_MT6761,   soc, 12)
  SOC_EQ(tmp, "MT6762D",  "Helio A25",       SOC_MTK_MT6762D,  soc, 12)
  //SOC_EQ(tmp, "?",      "Helio G25",       SOC_MTK_,         soc, 12)
  //SOC_EQ(tmp, "?",      "Helio G35",       SOC_MTK_,         soc, 12)
  //SOC_EQ(tmp, "?",      "Helio G70",       SOC_MTK_,         soc, 12)
  //SOC_EQ(tmp, "?",      "Helio G80",       SOC_MTK_,         soc, 12)
  //SOC_EQ(tmp, "?",      "Helio G90",       SOC_MTK_,         soc, 12)
  //SOC_EQ(tmp, "?",      "Helio G90T",      SOC_MTK_,         soc, 12)
  //SOC_EQ(tmp, "?",      "Helio G95",       SOC_MTK_,         soc, 12)
  SOC_EQ(tmp, "MT6755",   "Helio P10",       SOC_MTK_MT6755M,  soc, 28)
  SOC_EQ(tmp, "MT6755M",  "Helio P10 M",     SOC_MTK_MT6755M,  soc, 28)
  SOC_EQ(tmp, "MT6755T",  "Helio P15",       SOC_MTK_MT6755T,  soc, 28)
  SOC_EQ(tmp, "MT6757",   "Helio P20",       SOC_MTK_MT6757,   soc, 16)
  SOC_EQ(tmp, "MT6762",   "Helio P22",       SOC_MTK_MT6762,   soc, 12)
  SOC_EQ(tmp, "MT6763V",  "Helio P23",       SOC_MTK_MT6763V,  soc, 16)
  SOC_EQ(tmp, "MT6763T",  "Helio P23",       SOC_MTK_MT6763T,  soc, 16)
  SOC_EQ(tmp, "MT6757CD", "Helio P25",       SOC_MTK_MT6757CD, soc, 16)
  SOC_EQ(tmp, "MT6758",   "Helio P30",       SOC_MTK_MT6758,   soc, 16)
  SOC_EQ(tmp, "MT6765",   "Helio P35",       SOC_MTK_MT6765,   soc, 12)
  SOC_EQ(tmp, "MT6771",   "Helio P60",       SOC_MTK_MT6771,   soc, 12)
  SOC_EQ(tmp, "MT6768",   "Helio P65",       SOC_MTK_MT6768,   soc, 12)
  SOC_EQ(tmp, "MT6771T",  "Helio P70",       SOC_MTK_MT6771,   soc, 12)
  SOC_EQ(tmp, "MT6771V",  "Helio P70",       SOC_MTK_MT6771,   soc, 12)
  SOC_EQ(tmp, "MT6779",   "Helio P90",       SOC_MTK_MT6779,   soc, 12)
  //SOC_EQ(tmp, "?",      "Helio P95",       SOC_MTK_,         soc, 12)
  SOC_EQ(tmp, "MT6795",   "Helio X10",       SOC_MTK_MT6795,   soc, 28)
  SOC_EQ(tmp, "MT6795T",  "Helio X10 T",     SOC_MTK_MT6795,   soc, 28)
  SOC_EQ(tmp, "MT6797",   "Helio X20",       SOC_MTK_MT6797,   soc, 20)
  SOC_EQ(tmp, "MT6797M",  "Helio X20 M",     SOC_MTK_MT6797,   soc, 20)
  SOC_EQ(tmp, "MT6797D",  "Helio X23",       SOC_MTK_MT6797,   soc, 20)
  SOC_EQ(tmp, "MT6797T",  "Helio X25",       SOC_MTK_MT6797T,  soc, 20)
  SOC_EQ(tmp, "MT6797X",  "Helio X27",       SOC_MTK_MT6797X,  soc, 20)
  SOC_EQ(tmp, "MT6799",   "Helio X30",       SOC_MTK_MT6799,   soc, 10)
  // MT XXXX //
  SOC_EQ(tmp, "MT6515",   "MT6515",          SOC_MTK_MT6515,   soc, 40)
  SOC_EQ(tmp, "MT6516",   "MT6516",          SOC_MTK_MT6516,   soc, 65)
  SOC_EQ(tmp, "MT6517",   "MT6517",          SOC_MTK_MT6517,   soc, 40)
  SOC_EQ(tmp, "MT6572",   "MT6572",          SOC_MTK_MT6572,   soc, 28)
  SOC_EQ(tmp, "MT6572M",  "MT6572M",         SOC_MTK_MT6572M,  soc, 28)
  SOC_EQ(tmp, "MT6573",   "MT6573",          SOC_MTK_MT6573,   soc, 65)
  SOC_EQ(tmp, "MT6575",   "MT6575",          SOC_MTK_MT6575,   soc, 40)
  SOC_EQ(tmp, "MT6577",   "MT6577",          SOC_MTK_MT6577,   soc, 40)
  SOC_EQ(tmp, "MT6577T",  "MT6577T",         SOC_MTK_MT6577T,  soc, 40)
  SOC_EQ(tmp, "MT6580",   "MT6580",          SOC_MTK_MT6580,   soc, 28)
  SOC_EQ(tmp, "MT6582",   "MT6582",          SOC_MTK_MT6582,   soc, 28)
  SOC_EQ(tmp, "MT6582M",  "MT6582M",         SOC_MTK_MT6582M,  soc, 28)
  SOC_EQ(tmp, "MT6589",   "MT6589",          SOC_MTK_MT6589,   soc, 28)
  SOC_EQ(tmp, "MT6589T",  "MT6589T",         SOC_MTK_MT6589T,  soc, 28)
  SOC_EQ(tmp, "MT6592",   "MT6592",          SOC_MTK_MT6592,   soc, 28)
  SOC_EQ(tmp, "MT6595",   "MT6595",          SOC_MTK_MT6595,   soc, 28)
  SOC_EQ(tmp, "MT6732",   "MT6732",          SOC_MTK_MT6732,   soc, 28)
  SOC_EQ(tmp, "MT6735",   "MT6735",          SOC_MTK_MT6735,   soc, 28)
  SOC_EQ(tmp, "MT6735M",  "MT6735M",         SOC_MTK_MT6735M,  soc, 28)
  SOC_EQ(tmp, "MT6735P",  "MT6735P",         SOC_MTK_MT6735P,  soc, 28)
  SOC_EQ(tmp, "MT6737",   "MT6737",          SOC_MTK_MT6737,   soc, 28)
  SOC_EQ(tmp, "MT6737M",  "MT6737M",         SOC_MTK_MT6737M,  soc, 28)
  SOC_EQ(tmp, "MT6737T",  "MT6737T",         SOC_MTK_MT6737T,  soc, 28)
  SOC_EQ(tmp, "MT6739",   "MT6739",          SOC_MTK_MT6739,   soc, 28)
  SOC_EQ(tmp, "MT6750",   "MT6750",          SOC_MTK_MT6750,   soc, 28)
  SOC_EQ(tmp, "MT6750S",  "MT6750S",         SOC_MTK_MT6750S,  soc, 28)
  SOC_EQ(tmp, "MT6750T",  "MT6750T",         SOC_MTK_MT6750T,  soc, 28)
  SOC_EQ(tmp, "MT6752",   "MT6752",          SOC_MTK_MT6752,   soc, 28)
  SOC_EQ(tmp, "MT6753",   "MT6753",          SOC_MTK_MT6753,   soc, 28)
  SOC_EQ(tmp, "MT6850",   "MT6850",          SOC_MTK_MT6850,   soc, 28)
  SOC_EQ(tmp, "MT8121",   "MT8121",          SOC_MTK_MT8121,   soc, 40)
  SOC_EQ(tmp, "MT8125",   "MT8125",          SOC_MTK_MT8125,   soc, 40)
  SOC_EQ(tmp, "MT8127",   "MT8127",          SOC_MTK_MT8127,   soc, 32)
  SOC_EQ(tmp, "MT8135",   "MT8135",          SOC_MTK_MT8135,   soc, 28)
  SOC_EQ(tmp, "MT8163A",  "MT8163A",         SOC_MTK_MT8163A,  soc, 28)
  SOC_EQ(tmp, "MT8163B",  "MT8163B",         SOC_MTK_MT8163B,  soc, 28)
  SOC_EQ(tmp, "MT8167B",  "MT8167B",         SOC_MTK_MT8167B,  soc, 28)
  SOC_EQ(tmp, "MT8173",   "MT8173",          SOC_MTK_MT8173,   soc, 28)
  SOC_EQ(tmp, "MT8176",   "MT8176",          SOC_MTK_MT8176,   soc, 28)
  SOC_EQ(tmp, "MT8321",   "MT8321",          SOC_MTK_MT8321,   soc, 28)
  SOC_EQ(tmp, "MT8382",   "MT8382",          SOC_MTK_MT8382,   soc, 28)
  SOC_EQ(tmp, "MT8581",   "MT8581",          SOC_MTK_MT8581,   soc, 28)
  SOC_EQ(tmp, "MT8735",   "MT8735",          SOC_MTK_MT8735,   soc, 28)
  SOC_EQ(tmp, "MT8765B",  "MT8765B",         SOC_MTK_MT8765B,  soc, 28)
  SOC_EQ(tmp, "MT8783",   "MT8783",          SOC_MTK_MT8783,   soc, 28)
  SOC_END
}

/*
 * APQ: Application Processor Qualcomm
 * MSM: Mobile Station Modem
 * In a APQXXXX or MSMXXXX, the second digit represents:
 * *------------------*
 * | Value | Meaning  |
 * *------------------*
 * |     0 | No modem |
 * |     2 | HPSA+    |
 * |     6 | CDMA     |
 * |     9 | LTE      |
 * *------------------*
 * Ref: https://www.tomshardware.com/reviews/snapdragon-801-performance-xperia-z2,3777-2.html
 *      TWO-HEADED SNAPDRAGON TAKES FLIGHT By Linley Gwennap
 *
 * If Qualcomm official website reports the SoC name without the initial two or three SKU name,
 * we assume APQ if second number is 0, or MSM if second number is different than 0
 * 
 * All SoC names here have been retrieved from official Qualcomm resources. However, Linux kernel
 * and Android may report the SoC with slightly different. Therefore, this function needs some
 * rework (e.g, debug with http://specdevice.com/unmoderated.php?lang=en)
 */
bool match_qualcomm(char* soc_name, struct system_on_chip* soc) {
  char* tmp;
  char* soc_name_upper = toupperstr(soc_name);

  if((tmp = strstr(soc_name_upper, "MSM")) != NULL);
  else if((tmp = strstr(soc_name_upper, "SDM")) != NULL);
  else if((tmp = strstr(soc_name_upper, "APQ")) != NULL);
  else if((tmp = strstr(soc_name_upper, "SM")) != NULL);    
  else if((tmp = strstr(soc_name_upper, "QM")) != NULL);      
  else if((tmp = strstr(soc_name_upper, "QSD")) != NULL);
  else return false;
  
  SOC_START
  // Snapdragon S1 //
  SOC_EQ(tmp, "QSD8650",        "S1",        SOC_SNAPD_QSD8650,        soc, 65)
  SOC_EQ(tmp, "QSD8250",        "S1",        SOC_SNAPD_QSD8250,        soc, 65)
  SOC_EQ(tmp, "MSM7627",        "S1",        SOC_SNAPD_MSM7627,        soc, 65)
  SOC_EQ(tmp, "MSM7227",        "S1",        SOC_SNAPD_MSM7227,        soc, 65)
  SOC_EQ(tmp, "MSM7627A",       "S1",        SOC_SNAPD_MSM7627A,       soc, 45)
  SOC_EQ(tmp, "MSM7227A",       "S1",        SOC_SNAPD_MSM7227A,       soc, 45)
  SOC_EQ(tmp, "MSM7625",        "S1",        SOC_SNAPD_MSM7625,        soc, 65)
  SOC_EQ(tmp, "MSM7225",        "S1",        SOC_SNAPD_MSM7225,        soc, 65)
  SOC_EQ(tmp, "MSM7625A",       "S1",        SOC_SNAPD_MSM7625A,       soc, 45)
  SOC_EQ(tmp, "MSM7225A",       "S1",        SOC_SNAPD_MSM7225A,       soc, 45)
  // Snapdragon S2 //
  SOC_EQ(tmp, "MSM8655",        "S2",        SOC_SNAPD_MSM8655,        soc, 45) 
  SOC_EQ(tmp, "MSM8255",        "S2",        SOC_SNAPD_MSM8255,        soc, 45) 
  SOC_EQ(tmp, "APQ8055",        "S2",        SOC_SNAPD_APQ8055,        soc, 45)
  SOC_EQ(tmp, "MSM7630",        "S2",        SOC_SNAPD_MSM7630,        soc, 45)
  SOC_EQ(tmp, "MSM7230",        "S2",        SOC_SNAPD_MSM7230,        soc, 45)     
  // Snapdragon S3 //
  SOC_EQ(tmp, "MSM8660",        "S3",        SOC_SNAPD_MSM8660,        soc, 45) 
  SOC_EQ(tmp, "MSM8260",        "S3",        SOC_SNAPD_MSM8260,        soc, 45)   
  SOC_EQ(tmp, "APQ8060",        "S3",        SOC_SNAPD_APQ8060,        soc, 45) 
  // Snapdragon S4 //
  SOC_EQ(tmp, "MSM8225",        "S4 Play",   SOC_SNAPD_MSM8225,        soc, 45)
  SOC_EQ(tmp, "MSM8625",        "S4 Play",   SOC_SNAPD_MSM8625,        soc, 45)
  SOC_EQ(tmp, "APQ8060A",       "S4 Plus",   SOC_SNAPD_APQ8060A,       soc, 28) 
  SOC_EQ(tmp, "MSM8960",        "S4 Plus",   SOC_SNAPD_MSM8960,        soc, 28) 
  SOC_EQ(tmp, "MSM8260A",       "S4 Plus",   SOC_SNAPD_MSM8260A,       soc, 28) 
  SOC_EQ(tmp, "MSM8627",        "S4 Plus",   SOC_SNAPD_MSM8627,        soc, 28)   
  SOC_EQ(tmp, "MSM8227",        "S4 Plus",   SOC_SNAPD_MSM8227,        soc, 28)   
  SOC_EQ(tmp, "APQ8064",        "S4 Pro",    SOC_SNAPD_APQ8064,        soc, 28) 
  SOC_EQ(tmp, "MSM8960T",       "S4 Pro",    SOC_SNAPD_MSM8960T,       soc, 28) 
  // Snapdragon 2XX //
  SOC_EQ(tmp, "MSM8110",        "200",       SOC_SNAPD_MSM8110,        soc, 28) 
  SOC_EQ(tmp, "MSM8210",        "200",       SOC_SNAPD_MSM8210,        soc, 28) 
  SOC_EQ(tmp, "MSM8610",        "200",       SOC_SNAPD_MSM8610,        soc, 28) 
  SOC_EQ(tmp, "MSM8112",        "200",       SOC_SNAPD_MSM8112,        soc, 28) 
  SOC_EQ(tmp, "MSM8212",        "200",       SOC_SNAPD_MSM8212,        soc, 28) 
  SOC_EQ(tmp, "MSM8612",        "200",       SOC_SNAPD_MSM8612,        soc, 28) 
  SOC_EQ(tmp, "MSM8225Q",       "200",       SOC_SNAPD_MSM8225Q,       soc, 45) 
  SOC_EQ(tmp, "MSM8625Q",       "200",       SOC_SNAPD_MSM8625Q,       soc, 45) 
  SOC_EQ(tmp, "MSM8208",        "208",       SOC_SNAPD_MSM8208,        soc, 28) 
  SOC_EQ(tmp, "MSM8905",        "205",       SOC_SNAPD_MSM8905,        soc, 28) 
  SOC_EQ(tmp, "MSM8909",        "210 / 212", SOC_SNAPD_MSM8909,        soc, 28) // In the future, we can differenciate them using frequency
  SOC_EQ(tmp, "QM215",          "215",       SOC_SNAPD_QM215,          soc, 28)
  // Snapdragon 4XX //
  SOC_EQ(tmp, "APQ8028",        "400",       SOC_SNAPD_APQ8028,        soc, 28)
  SOC_EQ(tmp, "MSM8228",        "400",       SOC_SNAPD_MSM8228,        soc, 28)
  SOC_EQ(tmp, "MSM8628",        "400",       SOC_SNAPD_MSM8628,        soc, 28)
  SOC_EQ(tmp, "MSM8928",        "400",       SOC_SNAPD_MSM8928,        soc, 28)
  SOC_EQ(tmp, "MSM8926",        "400",       SOC_SNAPD_MSM8926,        soc, 28)
  SOC_EQ(tmp, "APQ8030AB",      "400",       SOC_SNAPD_APQ8030AB,      soc, 28)
  SOC_EQ(tmp, "MSM8226",        "400",       SOC_SNAPD_MSM8226,        soc, 28)
  SOC_EQ(tmp, "MSM8230AB",      "400",       SOC_SNAPD_MSM8230AB,      soc, 28)
  SOC_EQ(tmp, "MSM8626",        "400",       SOC_SNAPD_MSM8626,        soc, 28)
  SOC_EQ(tmp, "MSM8630",        "400",       SOC_SNAPD_MSM8630,        soc, 28)
  SOC_EQ(tmp, "MSM8630AB",      "400",       SOC_SNAPD_MSM8630AB,      soc, 28)
  SOC_EQ(tmp, "MSM8930",        "400",       SOC_SNAPD_MSM8930,        soc, 28)
  SOC_EQ(tmp, "MSM8930AB",      "400",       SOC_SNAPD_MSM8930AB,      soc, 28)
  SOC_EQ(tmp, "MSM8916",        "410 / 412", SOC_SNAPD_MSM8916,        soc, 28)
  SOC_EQ(tmp, "MSM8929",        "415",       SOC_SNAPD_MSM8929,        soc, 28)
  SOC_EQ(tmp, "MSM8917",        "425",       SOC_SNAPD_MSM8917,        soc, 28)
  SOC_EQ(tmp, "MSM8920",        "427",       SOC_SNAPD_MSM8920,        soc, 28)
  SOC_EQ(tmp, "SDM429",         "429",       SOC_SNAPD_SDM429,         soc, 12)
  SOC_EQ(tmp, "MSM8937",        "430",       SOC_SNAPD_MSM8937,        soc, 28)
  SOC_EQ(tmp, "MSM8940",        "435",       SOC_SNAPD_MSM8940,        soc, 28)
  SOC_EQ(tmp, "SDM439",         "439",       SOC_SNAPD_SDM439,         soc, 12)
  SOC_EQ(tmp, "SDM450",         "450",       SOC_SNAPD_SDM450,         soc, 14)
  SOC_EQ(tmp, "SM4250-AA",      "460",       SOC_SNAPD_SM4250_AA,      soc, 11)
  // Snapdragon 6XX //
  SOC_EQ(tmp, "APQ8064T",       "600",       SOC_SNAPD_APQ8064T,       soc, 28)
  SOC_EQ(tmp, "APQ8064M",       "600",       SOC_SNAPD_APQ8064M,       soc, 28)
  SOC_EQ(tmp, "MSM8936",        "610",       SOC_SNAPD_MSM8936,        soc, 28)
  SOC_EQ(tmp, "MSM8939",        "615 / 616", SOC_SNAPD_MSM8939,        soc, 28)
  SOC_EQ(tmp, "MSM8952",        "617",       SOC_SNAPD_MSM8952,        soc, 28)
  SOC_EQ(tmp, "MSM8953",        "625",       SOC_SNAPD_MSM8953,        soc, 14)
  SOC_EQ(tmp, "MSM8953 PRO",    "626",       SOC_SNAPD_MSM8953_PRO,    soc, 14)
  SOC_EQ(tmp, "SDM630",         "630",       SOC_SNAPD_SDM630,         soc, 14)
  SOC_EQ(tmp, "SDM632",         "632",       SOC_SNAPD_SDM632,         soc, 14)
  SOC_EQ(tmp, "SDM636",         "636",       SOC_SNAPD_SDM636,         soc, 14)
  SOC_EQ(tmp, "MSM8956",        "650",       SOC_SNAPD_MSM8956,        soc, 28)
  SOC_EQ(tmp, "MSM8976",        "652",       SOC_SNAPD_MSM8976,        soc, 28)
  SOC_EQ(tmp, "MSM8976 PRO",    "653",       SOC_SNAPD_MSM8976_PRO,    soc, 28)
  SOC_EQ(tmp, "SDM660",         "660",       SOC_SNAPD_SDM660,         soc, 14)
  SOC_EQ(tmp, "SM6115",         "662",       SOC_SNAPD_SM6115,         soc, 11)
  SOC_EQ(tmp, "SM6125",         "665",       SOC_SNAPD_SM6125,         soc, 11)
  SOC_EQ(tmp, "SDM670",         "670",       SOC_SNAPD_SDM670,         soc, 10)
  SOC_EQ(tmp, "SM6150",         "675",       SOC_SNAPD_SM6150,         soc, 11)
  SOC_EQ(tmp, "SM6350",         "690",       SOC_SNAPD_SM6350,         soc,  8)
  // Snapdragon 7XX //
  SOC_EQ(tmp, "SDM710",         "710",       SOC_SNAPD_SDM710,         soc, 10)
  SOC_EQ(tmp, "SDM712",         "712",       SOC_SNAPD_SDM712,         soc, 10)
  SOC_EQ(tmp, "SM7125",         "720G",      SOC_SNAPD_SM7125,         soc,  8)
  SOC_EQ(tmp, "SM7150-AA",      "730",       SOC_SNAPD_SM7150_AA,      soc,  8)
  SOC_EQ(tmp, "SM7150-AB",      "730G",      SOC_SNAPD_SM7150_AB,      soc,  8)
  SOC_EQ(tmp, "SM7150-AC",      "732G",      SOC_SNAPD_SM7150_AC,      soc,  8)
  SOC_EQ(tmp, "SM7225",         "750G",      SOC_SNAPD_SM7225,         soc,  8)
  SOC_EQ(tmp, "SM7250-AA",      "765",       SOC_SNAPD_SM7250_AA,      soc,  7)
  SOC_EQ(tmp, "SM7250-AB",      "765G",      SOC_SNAPD_SM7250_AB,      soc,  7)
  SOC_EQ(tmp, "SM7250-AC",      "768G",      SOC_SNAPD_SM7250_AC,      soc,  7)
  // Snapdragon 8XX //
  SOC_EQ(tmp, "MSM8974AA",      "800",       SOC_SNAPD_MSM8974AA,      soc, 28)
  SOC_EQ(tmp, "MSM8974AB",      "800",       SOC_SNAPD_MSM8974AB,      soc, 28)
  SOC_EQ(tmp, "MSM8974AC",      "800",       SOC_SNAPD_MSM8974AC,      soc, 28)
  SOC_EQ(tmp, "MSM8974PRO-AB",  "801",       SOC_SNAPD_MSM8974PRO_AB,  soc, 28)
  SOC_EQ(tmp, "MSM8974PRO-AC",  "801",       SOC_SNAPD_MSM8974PRO_AC,  soc, 28)
  SOC_EQ(tmp, "APQ8084",        "805",       SOC_SNAPD_APQ8084,        soc, 28)
  SOC_EQ(tmp, "MSM8992",        "808",       SOC_SNAPD_MSM8992,        soc, 20)
  SOC_EQ(tmp, "MSM8994",        "810",       SOC_SNAPD_MSM8994,        soc, 20)
  SOC_EQ(tmp, "MSM8996",        "820",       SOC_SNAPD_MSM8996,        soc, 14)
  SOC_EQ(tmp, "MSM8996 PRO A",  "821",       SOC_SNAPD_MSM8996_PRO_A,  soc, 14)
  SOC_EQ(tmp, "MSM8998",        "835",       SOC_SNAPD_MSM8998,        soc, 10)
  SOC_EQ(tmp, "APQ8098",        "835",       SOC_SNAPD_APQ8098,        soc, 10)
  SOC_EQ(tmp, "SDM845",         "845",       SOC_SNAPD_SDM845,         soc, 10)
  SOC_EQ(tmp, "SDM850",         "850",       SOC_SNAPD_SDM850,         soc, 10)
  SOC_EQ(tmp, "SM8150",         "855",       SOC_SNAPD_SM8150,         soc,  7)
  SOC_EQ(tmp, "SM8150-AC",      "855+",      SOC_SNAPD_SM8150_AC,      soc,  7)
  SOC_EQ(tmp, "SM8250",         "865",       SOC_SNAPD_SM8250,         soc,  7)
  SOC_EQ(tmp, "SM8250-AB",      "865+",      SOC_SNAPD_SM8250_AB,      soc,  7)
  SOC_EQ(tmp, "SM8350",         "888",       SOC_SNAPD_SM8350,         soc,  5)
  SOC_END
}

bool match_special(char* soc_name, struct system_on_chip* soc) {
  char* tmp;

  // Xiaomi hides Redmi Note 8/8T under "Qualcomm Technologies, Inc TRINKET"
  if((tmp = strstr(soc_name, "TRINKET")) != NULL) {
    fill_soc(soc, "665", SOC_SNAPD_SM6125, 11);
    return true;
  }

  // Snapdragon 730 reported as "Qualcomm Technologies, Inc. SDMMAGPIE"
  if((tmp = strstr(soc_name, "SDMMAGPIE")) != NULL) {
    fill_soc(soc, "730", SOC_SNAPD_SM7150_AA, 8);
    return true;
  }

  return false;
}

struct system_on_chip* parse_soc_from_string(struct system_on_chip* soc) {
  char* raw_name = soc->raw_name;

  if(match_special(raw_name, soc))
    return soc;
  
  if (match_qualcomm(raw_name, soc))
    return soc;
  
  if(match_mediatek(raw_name, soc))
    return soc;
  
  if(match_exynos(raw_name, soc))
    return soc;
  
  if(match_hisilicon(raw_name, soc))
    return soc;
  
  match_broadcom(raw_name, soc);
  
  return soc;
}

#ifdef __ANDROID__
#include <sys/system_properties.h>

static inline int android_property_get(const char* key, char* value) {
  return __system_property_get(key, value);
}

struct system_on_chip* guess_soc_from_android(struct system_on_chip* soc) {
  char tmp[100];
  int property_len = 0;
  
  property_len = android_property_get("ro.mediatek.platform", (char *) &tmp);
  if(property_len > 0) {
    soc->raw_name = malloc(sizeof(char) * (property_len + 1));
    strncpy(soc->raw_name, tmp, property_len + 1);
    soc->raw_name[property_len] = '\0';
    soc->soc_vendor = SOC_VENDOR_UNKNOWN;
    return parse_soc_from_string(soc);
  }
  
  property_len = android_property_get("ro.product.board", (char *) &tmp);
  if(property_len > 0) {    
    soc->raw_name = malloc(sizeof(char) * (property_len + 1));
    strncpy(soc->raw_name, tmp, property_len + 1);
    soc->raw_name[property_len] = '\0';
    soc->soc_vendor = SOC_VENDOR_UNKNOWN;
    return parse_soc_from_string(soc);
  }    
  
  return soc;
}
#endif

struct system_on_chip* guess_soc_from_cpuinfo(struct system_on_chip* soc) {
  char* tmp = get_hardware_from_cpuinfo();
  
  if(tmp != NULL) {
    soc->raw_name = tmp;
    return parse_soc_from_string(soc);
  }
  
  return soc;
}

int hex2int(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;

  return -1;
}

// https://www.raspberrypi.org/documentation/hardware/raspberrypi/revision-codes/README.md
struct system_on_chip* guess_soc_raspbery_pi(struct system_on_chip* soc) {
  char* revision = get_revision_from_cpuinfo();

  if(revision == NULL) {
    printWarn("[RPi] Couldn't find revision field in cpuinfo");
    return soc;
  }

  if(strlen(revision) != 6) {
    printWarn("[RPi] Found invalid RPi revision code: '%s'", revision);
    return soc;
  }

  int arr_size = ARRAY_SIZE(soc_rpi_string);
  int pppp = hex2int(revision[2]);
  if(pppp == -1) {
    printErr("[RPi] Found invalid RPi PPPP code: %s", revision[2]);
    return soc;
  }

  if(pppp > arr_size) {
    printErr("[RPi] Found invalid RPi PPPP code: %d while max is %d", pppp, arr_size);
    return soc;
  }

  char* soc_raw_name = soc_rpi_string[pppp];
  /*int soc_len = strlen(soc_raw_name);
  soc->raw_name = malloc(sizeof(char) * (soc_len + 1));
  strncpy(soc->raw_name, soc_raw_name, soc_len + 1);*/

  match_broadcom(soc_raw_name, soc);
  return soc;
}

struct system_on_chip* get_soc() {
  struct system_on_chip* soc = malloc(sizeof(struct system_on_chip));
  soc->raw_name = NULL;
  soc->soc_vendor = SOC_VENDOR_UNKNOWN;
  soc->process = UNKNOWN;

  bool isRPi = is_raspberry_pi();
  if(isRPi) {
    soc = guess_soc_raspbery_pi(soc);
    if(soc->soc_vendor == SOC_VENDOR_UNKNOWN) {
      printWarn("SoC detection failed using revision code");
    }
    else {
      return soc;
    }
  }

  soc = guess_soc_from_cpuinfo(soc);
  if(soc->soc_vendor == SOC_VENDOR_UNKNOWN) {
    if(soc->raw_name != NULL)
      printWarn("SoC detection failed using /proc/cpuinfo: Found '%s' string", soc->raw_name);   
    else
      printWarn("SoC detection failed using /proc/cpuinfo: No string found");
#ifdef __ANDROID__
    soc = guess_soc_from_android(soc);
    if(soc->raw_name == NULL)
      printWarn("SoC detection failed using Android: No string found");
    else if(soc->soc_vendor == SOC_VENDOR_UNKNOWN)
      printWarn("SoC detection failed using Android: Found '%s' string", soc->raw_name);   
#endif
  }

  if(soc->raw_name == NULL) {
    soc->raw_name = malloc(sizeof(char) * (strlen(STRING_UNKNOWN)+1));
    snprintf(soc->raw_name, strlen(STRING_UNKNOWN)+1, STRING_UNKNOWN);
  }
  
  return soc;    
}

char* get_soc_name(struct system_on_chip* soc) {
  if(soc->soc_vendor == SOC_VENDOR_UNKNOWN)
    return soc->raw_name;
  return soc->soc_name;
}

VENDOR get_soc_vendor(struct system_on_chip* soc) {
  return soc->soc_vendor;
}

char* get_str_process(struct system_on_chip* soc) {
  char* str;
  
  if(soc->process == UNKNOWN) {
    str = malloc(sizeof(char) * (strlen(STRING_UNKNOWN)+1));
    snprintf(str, strlen(STRING_UNKNOWN)+1, STRING_UNKNOWN);
  }
  else {
    str = malloc(sizeof(char) * 5);
    memset(str, 0, sizeof(char) * 5);
    snprintf(str, 5, "%dnm", soc->process);    
  }
  return str;
}

