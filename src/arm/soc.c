#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "soc.h"
#include "socs.h"
#include "udev.h"
#include "uarch.h"
#include "../common/global.h"
#include "../common/pci.h"

#if defined(__APPLE__) || defined(__MACH__)
  #include "../common/sysctl.h"
#endif

#define NA -1
#define min(a,b) (((a)<(b))?(a):(b))
#define ARRAY_SIZE(arr)     (sizeof(arr) / sizeof((arr)[0]))

#define PROP_MTK_PLATFORM   "ro.mediatek.platform"
#define PROP_SOC_MODEL      "ro.soc.model"
#define PROP_PRODUCT_BOARD  "ro.product.board"
#define PROP_BOARD_PLATFORM "ro.board.platform"

static char* soc_rpi_string[] = {
  "BCM2835",
  "BCM2836",
  "BCM2837",
  "BCM2711",
  "BCM2712"
};

char* toupperstr(char* str) {
  int len = strlen(str) + 1;
  char* ret = emalloc(sizeof(char) * len);
  memset(ret, 0, sizeof(char) * len);

  for(int i=0; i < len; i++) {
    ret[i] = toupper((unsigned char) str[i]);
  }

  return ret;
}

uint32_t get_sid_from_nvmem(char* buf) {
  uint32_t sid = 0;
  sid += (unsigned char) buf[0] * 1<<0;
  sid += (unsigned char) buf[1] * 1<<8;
  sid += (unsigned char) buf[2] * 1<<16;
  sid += (unsigned char) buf[3] * 1<<24;
  return sid;
}

// Security ID (SID) is an identifier that can help with SoC detection
// in Allwinner SoCs (https://linux-sunxi.org/SID_Register_Guide#Security_ID)
// SIDs list:
// - https://linux-sunxi.org/SID_Register_Guide#Currently_known_SID.27s
// - https://github.com/Dr-Noob/cpufetch/issues/173
// - https://github.com/ThomasKaiser/sbc-bench/blob/master/sbc-bench.sh
// - https://linux-sunxi.org/*CHIP_NAME*
bool get_sunxisoc_from_sid(struct system_on_chip* soc, char* raw_name, uint32_t sid) {
  typedef struct {
    uint32_t sid;
    struct system_on_chip soc;
  } sidToSoC;

  sidToSoC socFromSid[] = {
    // --- sun8i Family ---
    // A33
    {0x0461872a, {SOC_ALLWINNER_A33,    SOC_VENDOR_ALLWINNER, 40, "A33",  raw_name} },
    // A83T
    {0x32c00401, {SOC_ALLWINNER_A83T,   SOC_VENDOR_ALLWINNER, 28, "A83T", raw_name} },
    {0x32c00403, {SOC_ALLWINNER_A83T,   SOC_VENDOR_ALLWINNER, 28, "A83T", raw_name} },
    // S3
    {0x12c00001, {SOC_ALLWINNER_S3,     SOC_VENDOR_ALLWINNER, 40, "S3",   raw_name} },
    // H2+
    {0x02c00042, {SOC_ALLWINNER_H2PLUS, SOC_VENDOR_ALLWINNER, 40, "H2+",  raw_name} },
    {0x02c00142, {SOC_ALLWINNER_H2PLUS, SOC_VENDOR_ALLWINNER, 40, "H2+",  raw_name} },
    {0x02c00242, {SOC_ALLWINNER_H2PLUS, SOC_VENDOR_ALLWINNER, 40, "H2+",  raw_name} },
    // H3
    {0x02c00181, {SOC_ALLWINNER_H3,     SOC_VENDOR_ALLWINNER, 40, "H3",   raw_name} },
    {0x02c00081, {SOC_ALLWINNER_H3,     SOC_VENDOR_ALLWINNER, 40, "H3",   raw_name} },
    // R40
    {0x12c00017, {SOC_ALLWINNER_R40,    SOC_VENDOR_ALLWINNER, 40, "R40",  raw_name} },
    // V3S
    {0x12c00000, {SOC_ALLWINNER_V3S,    SOC_VENDOR_ALLWINNER, 40, "V3s",  raw_name} }, // 40nm is only my guess, no source
    // --- sun50i Family ---
    // H5
    {0x82800001, {SOC_ALLWINNER_H5,     SOC_VENDOR_ALLWINNER, 40, "H5",   raw_name} },
    // H6
    {0x82c00001, {SOC_ALLWINNER_H6,     SOC_VENDOR_ALLWINNER, 28, "H6",   raw_name} },
    {0x82c00007, {SOC_ALLWINNER_H6,     SOC_VENDOR_ALLWINNER, 28, "H6",   raw_name} },
    // H64
    {0x92c000bb, {SOC_ALLWINNER_H64,    SOC_VENDOR_ALLWINNER, 40, "H64",  raw_name} }, // Same manufacturing process as A64
    // H616
    {0x32c05000, {SOC_ALLWINNER_H616,   SOC_VENDOR_ALLWINNER, 28, "H616", raw_name} },
    // H618
    {0x33802000, {SOC_ALLWINNER_H618,   SOC_VENDOR_ALLWINNER, 28, "H618", raw_name} },
    // A64
    {0x92c000ba, {SOC_ALLWINNER_A64,    SOC_VENDOR_ALLWINNER, 40, "A64",  raw_name} },
    {0x92c001ba, {SOC_ALLWINNER_A64,    SOC_VENDOR_ALLWINNER, 40, "A64",  raw_name} },
    // Unknown
    {0x00000000, {UNKNOWN,              SOC_VENDOR_UNKNOWN,   -1, "",     raw_name} }
  };

  int index = 0;
  while(socFromSid[index].sid != 0x0) {
    if(socFromSid[index].sid == sid) {
      fill_soc(soc, socFromSid[index].soc.name, socFromSid[index].soc.model, socFromSid[index].soc.process);
      return true;
    }
    index++;
  }

  printErr("SID was found but it does not match any known SIDs: %08x", sid);
  return false;
}

// Exynos special define (not included in src/common/soc.h)
#define SOC_EXY_EQ(raw_name, tmpsoc, soc_name, soc_model, soc, process)             \
   sprintf(tmpsoc, "exynos%s", soc_name);                                           \
   if (match_soc(soc, raw_name, tmpsoc, soc_name, soc_model, process)) return true; \
   sprintf(tmpsoc, "universal%s", soc_name);                                        \
   if (match_soc(soc, raw_name, tmpsoc, soc_name, soc_model, process)) return true;

// https://en.wikipedia.org/wiki/Raspberry_Pi
// http://phonedb.net/index.php?m=processor&id=562&c=broadcom_bcm21663
// https://hwbot.org/hardware/processors#key=bcmxxx
bool match_broadcom(char* soc_name, struct system_on_chip* soc) {
  char* tmp;

  if((tmp = strstr(soc_name, "BCM")) == NULL)
    return false;

  soc->vendor = SOC_VENDOR_BROADCOM;

  SOC_START
  SOC_EQ(tmp, "BCM2835",              "BCM2835",              SOC_BCM_2835,   soc, 65)
  SOC_EQ(tmp, "BCM2836",              "BCM2836",              SOC_BCM_2836,   soc, 40)
  SOC_EQ(tmp, "BCM2837",              "BCM2837",              SOC_BCM_2837,   soc, 40)
  SOC_EQ(tmp, "BCM2837B0",            "BCM2837B0",            SOC_BCM_2837B0, soc, 40)
  SOC_EQ(tmp, "BCM21553",             "BCM21553",             SOC_BCM_21553,  soc, 65)
  SOC_EQ(tmp, "BCM21553-Thunderbird", "BCM21553 Thunderbird", SOC_BCM_21553T, soc, 65)
  SOC_EQ(tmp, "BCM21663",             "BCM21663",             SOC_BCM_21663,  soc, 40)
  SOC_EQ(tmp, "BCM21664",             "BCM21664",             SOC_BCM_21664,  soc, 40)
  SOC_EQ(tmp, "BCM28155",             "BCM28155",             SOC_BCM_28155,  soc, 40)
  SOC_EQ(tmp, "BCM23550",             "BCM23550",             SOC_BCM_23550,  soc, 40)
  SOC_EQ(tmp, "BCM28145",             "BCM28145",             SOC_BCM_28145,  soc, 40)
  SOC_EQ(tmp, "BCM2157",              "BCM2157",              SOC_BCM_2157,   soc, 65)
  SOC_EQ(tmp, "BCM21654",             "BCM21654",             SOC_BCM_21654,  soc, 40)
  SOC_EQ(tmp, "BCM2711",              "BCM2711",              SOC_BCM_2711,   soc, 28)
  SOC_EQ(tmp, "BCM2712",              "BCM2712",              SOC_BCM_2712,   soc, 16)
  SOC_END
}

// https://en.wikipedia.org/wiki/Google_Tensor
bool match_google(char* soc_name, struct system_on_chip* soc) {
  char* tmp;

  if((tmp = strstr(soc_name, "gs")) == NULL)
    return false;

  soc->vendor = SOC_VENDOR_GOOGLE;

  SOC_START
  SOC_EQ(tmp, "gs101", "Tensor",    SOC_GOOGLE_TENSOR,    soc, 5)
  SOC_EQ(tmp, "gs201", "Tensor G2", SOC_GOOGLE_TENSOR_G2, soc, 5)
  SOC_EQ(tmp, "gs301", "Tensor G3", SOC_GOOGLE_TENSOR_G3, soc, 4)
  SOC_END
}

// https://www.techinsights.com/
// https://datasheetspdf.com/pdf-file/1316605/HiSilicon/Hi3660/1
// https://github.com/Dr-Noob/cpufetch/issues/259
bool match_hisilicon(char* soc_name, struct system_on_chip* soc) {
  char* tmp;

  if((tmp = strstr(soc_name, "hi")) != NULL);
  else if((tmp = strstr(soc_name, "kirin")) != NULL);
  else return false;

  soc->vendor = SOC_VENDOR_KIRIN;

  SOC_START
  SOC_EQ(tmp, "hi3620GFC",  "K3V2",  SOC_HISILICON_3620, soc, 40)
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
  SOC_EQ(tmp, "hi3630",     "920",   SOC_HISILICON_3630, soc, 28)
  //SOC_EQ(tmp, "?",        "925",   SOC_KIRIN, soc, 28)
  //SOC_EQ(tmp, "?",        "930",   SOC_KIRIN, soc, ?)
  //SOC_EQ(tmp, "?",        "935",   SOC_KIRIN, soc, ?)
  SOC_EQ(tmp, "hi3650",     "950",   SOC_HISILICON_3650, soc, 16)
  //SOC_EQ(tmp, "?",        "955",   SOC_KIRIN, soc, ?)
  SOC_EQ(tmp, "hi3660",     "960",   SOC_HISILICON_3660, soc, 16)
  //SOC_EQ(tmp, "?",        "960S",  SOC_KIRIN, soc, 16)
  SOC_EQ(tmp, "hi3670",     "970",   SOC_HISILICON_3670, soc, 10)
  SOC_EQ(tmp, "hi3680",     "980",   SOC_HISILICON_3680, soc,  7)
  //SOC_EQ(tmp, "?",        "985",   SOC_KIRIN, soc,  7)
  SOC_EQ(tmp, "hi3690",     "990",   SOC_HISILICON_3690, soc,  7)
  SOC_EQ(tmp, "kirin9000s", "9000s", SOC_HISILICON_9000S,soc,  7)
  SOC_END
}

bool match_exynos(char* soc_name, struct system_on_chip* soc) {
  char* tmp;

  if((tmp = strstr(soc_name, "universal")) != NULL);
  else if((tmp = strstr(soc_name, "exynos")) != NULL);
  else return false;

  soc->vendor = SOC_VENDOR_EXYNOS;

  // Because exynos are recently using "exynosXXXX" instead
  // of "universalXXXX" as codenames, SOC_EXY_EQ will check for
  // both cases, since it seems that there are some SoCs that
  // can appear with both codenames

  // Used by SOC_EXY_EQ
  char tmpsoc[14];

  SOC_START
  SOC_EXY_EQ(tmp, tmpsoc, "3475", SOC_EXYNOS_3475, soc, 28)
  SOC_EXY_EQ(tmp, tmpsoc, "4210", SOC_EXYNOS_4210, soc, 45)
  SOC_EXY_EQ(tmp, tmpsoc, "4212", SOC_EXYNOS_4212, soc, 32)
  SOC_EXY_EQ(tmp, tmpsoc, "4412", SOC_EXYNOS_4412, soc, 32)
  SOC_EXY_EQ(tmp, tmpsoc, "5250", SOC_EXYNOS_5250, soc, 32)
  SOC_EXY_EQ(tmp, tmpsoc, "5410", SOC_EXYNOS_5410, soc, 28)
  SOC_EXY_EQ(tmp, tmpsoc, "5420", SOC_EXYNOS_5420, soc, 28)
  SOC_EXY_EQ(tmp, tmpsoc, "5422", SOC_EXYNOS_5422, soc, 28)
  SOC_EXY_EQ(tmp, tmpsoc, "5430", SOC_EXYNOS_5430, soc, 20)
  SOC_EXY_EQ(tmp, tmpsoc, "5433", SOC_EXYNOS_5433, soc, 20)
  SOC_EXY_EQ(tmp, tmpsoc, "5260", SOC_EXYNOS_5260, soc, 28)
  SOC_EXY_EQ(tmp, tmpsoc, "7270", SOC_EXYNOS_7270, soc, 14)
  SOC_EXY_EQ(tmp, tmpsoc, "7420", SOC_EXYNOS_7420, soc, 14)
  SOC_EXY_EQ(tmp, tmpsoc, "7570", SOC_EXYNOS_7570, soc, 14)
  SOC_EXY_EQ(tmp, tmpsoc, "7570", SOC_EXYNOS_7570, soc, 14)
  SOC_EXY_EQ(tmp, tmpsoc, "7870", SOC_EXYNOS_7870, soc, 14)
  SOC_EXY_EQ(tmp, tmpsoc, "7870", SOC_EXYNOS_7870, soc, 14)
  SOC_EXY_EQ(tmp, tmpsoc, "7872", SOC_EXYNOS_7872, soc, 14)
  SOC_EXY_EQ(tmp, tmpsoc, "7880", SOC_EXYNOS_7880, soc, 14)
  SOC_EXY_EQ(tmp, tmpsoc, "7884", SOC_EXYNOS_7884, soc, 14)
  SOC_EXY_EQ(tmp, tmpsoc, "7885", SOC_EXYNOS_7885, soc, 14)
  SOC_EXY_EQ(tmp, tmpsoc, "7904", SOC_EXYNOS_7904, soc, 14)
  SOC_EXY_EQ(tmp, tmpsoc, "8890", SOC_EXYNOS_8890, soc, 14)
  SOC_EXY_EQ(tmp, tmpsoc, "8895", SOC_EXYNOS_8895, soc, 10)
  SOC_EXY_EQ(tmp, tmpsoc, "9110", SOC_EXYNOS_9110, soc, 14)
  SOC_EXY_EQ(tmp, tmpsoc, "9609", SOC_EXYNOS_9609, soc, 10)
  SOC_EXY_EQ(tmp, tmpsoc, "9610", SOC_EXYNOS_9610, soc, 10)
  SOC_EXY_EQ(tmp, tmpsoc, "9611", SOC_EXYNOS_9611, soc, 10)
  SOC_EXY_EQ(tmp, tmpsoc, "9810", SOC_EXYNOS_9810, soc, 10)
  SOC_EXY_EQ(tmp, tmpsoc, "9820", SOC_EXYNOS_9820, soc,  8)
  SOC_EXY_EQ(tmp, tmpsoc, "9825", SOC_EXYNOS_9825, soc,  7)
  SOC_EXY_EQ(tmp, tmpsoc, "1080", SOC_EXYNOS_1080, soc,  5)
  SOC_EXY_EQ(tmp, tmpsoc, "990",  SOC_EXYNOS_990,  soc,  7)
  SOC_EXY_EQ(tmp, tmpsoc, "980",  SOC_EXYNOS_980,  soc,  8)
  SOC_EXY_EQ(tmp, tmpsoc, "880",  SOC_EXYNOS_880,  soc,  8)
  SOC_END
}

// https://www.phonemore.com/processors/mediatek/
// https://phonedb.net/
// https://en.wikipedia.org/wiki/List_of_MediaTek_systems_on_chips
// https://wikimovel.com/index.php/MediaTek
bool match_mediatek(char* soc_name, struct system_on_chip* soc) {
  char* tmp;
  char* soc_name_upper = toupperstr(soc_name);

  if((tmp = strstr(soc_name_upper, "MT")) == NULL)
    return false;

  soc->vendor = SOC_VENDOR_MEDIATEK;

  SOC_START
  // Dimensity //
  SOC_EQ(tmp, "MT6893Z",  "Dimensity 1300",  SOC_MTK_MT6893Z,  soc, 6)
  SOC_EQ(tmp, "MT6893",   "Dimensity 1200",  SOC_MTK_MT6893,   soc, 6)
  SOC_EQ(tmp, "MT6891",   "Dimensity 1100",  SOC_MTK_MT6891,   soc, 6)
  //SOC_EQ(tmp, "MT6877V",  "Dimensity 1080",  SOC_MTK_MT6877V   soc, 7) // There is a clash between this and another chip
  SOC_EQ(tmp, "MT6879",   "Dimensity 1050",  SOC_MTK_MT6879,   soc, 6)
  SOC_EQ(tmp, "MT6889",   "Dimensity 1000",  SOC_MTK_MT6889,   soc, 7)
  SOC_EQ(tmp, "MT6885Z",  "Dimensity 1000L", SOC_MTK_MT6885Z,  soc, 7)
  SOC_EQ(tmp, "MT6889Z",  "Dimensity 1000+", SOC_MTK_MT6889Z,  soc, 7)
  SOC_EQ(tmp, "MT6883Z",  "Dimensity 1000C", SOC_MTK_MT6883Z,  soc, 7)
  SOC_EQ(tmp, "MT6833",   "Dimensity 700",   SOC_MTK_MT6833,   soc, 7)
  SOC_EQ(tmp, "MT6853",   "Dimensity 720",   SOC_MTK_MT6853,   soc, 7)
  SOC_EQ(tmp, "MT6873",   "Dimensity 800",   SOC_MTK_MT6873,   soc, 7)
  SOC_EQ(tmp, "MT6853V",  "Dimensity 800U",  SOC_MTK_MT6853V,  soc, 7)
  SOC_EQ(tmp, "MT6833",   "Dimensity 810",   SOC_MTK_MT6833,   soc, 6)
  SOC_EQ(tmp, "MT6875",   "Dimensity 820",   SOC_MTK_MT6875,   soc, 7)
  // Helio //
  SOC_EQ(tmp, "MT6761D",  "Helio A20",       SOC_MTK_MT6761D,  soc, 12)
  SOC_EQ(tmp, "MT6761",   "Helio A22",       SOC_MTK_MT6761,   soc, 12)
  SOC_EQ(tmp, "MT6762D",  "Helio A25",       SOC_MTK_MT6762D,  soc, 12)
  SOC_EQ(tmp, "MT6762G",  "Helio G25",       SOC_MTK_MT6762G,  soc, 12)
  SOC_EQ(tmp, "MT6765G",  "Helio G35",       SOC_MTK_MT6765G,  soc, 12)
  //SOC_EQ(tmp, "???",    "Helio G36",       SOC_MTK_MT6765G,  soc,  ?)
  SOC_EQ(tmp, "MT6765H",  "Helio G37",       SOC_MTK_MT6765H,  soc, 12)
  SOC_EQ(tmp, "MT6769V",  "Helio G70",       SOC_MTK_MT6769V,  soc, 12)
  SOC_EQ(tmp, "MT6769T",  "Helio G80",       SOC_MTK_MT6769T,  soc, 12)
  SOC_EQ(tmp, "MT6769Z",  "Helio G85",       SOC_MTK_MT6769Z,  soc, 12)
  SOC_EQ(tmp, "MT6769H",  "Helio G88",       SOC_MTK_MT6769H,  soc, 12)
  //SOC_EQ(tmp, "MT6785V/CD", "Helio G90",   SOC_MTK_MT6785V_CD, soc, 12) // How to distingish between this and G95?
  SOC_EQ(tmp, "MT6785V/CC", "Helio G90T",    SOC_MTK_MT6785V_CC, soc, 12)
  SOC_EQ(tmp, "MT6785V/CD", "Helio G95",     SOC_MTK_MT6785V_CD, soc, 12)
  SOC_EQ(tmp, "MT6789",   "Helio G99",       SOC_MTK_MT6789,   soc,  6)
  SOC_EQ(tmp, "MT8781V",  "Helio G99",       SOC_MTK_MT8781V,  soc,  6) // Same as MT6789
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
  SOC_EQ(tmp, "MT6779V/CU", "Helio P90",     SOC_MTK_MT6779V_CU, soc, 12)
  SOC_EQ(tmp, "MT6779V/CV", "Helio P95",     SOC_MTK_MT6779V_CV, soc, 12)
  SOC_EQ(tmp, "MT6795",   "Helio X10",       SOC_MTK_MT6795,   soc, 28)
  SOC_EQ(tmp, "MT6795T",  "Helio X10 T",     SOC_MTK_MT6795,   soc, 28)
  SOC_EQ(tmp, "MT6797",   "Helio X20",       SOC_MTK_MT6797,   soc, 20)
  SOC_EQ(tmp, "MT6797M",  "Helio X20 M",     SOC_MTK_MT6797,   soc, 20)
  SOC_EQ(tmp, "MT6797D",  "Helio X23",       SOC_MTK_MT6797,   soc, 20)
  SOC_EQ(tmp, "MT6797T",  "Helio X25",       SOC_MTK_MT6797T,  soc, 20)
  SOC_EQ(tmp, "MT6797X",  "Helio X27",       SOC_MTK_MT6797X,  soc, 20)
  SOC_EQ(tmp, "MT6799",   "Helio X30",       SOC_MTK_MT6799,   soc, 10)
  // Pentonic
  SOC_EQ(tmp, "MT9618",   "Pentonic  700",   SOC_MTK_MT9618,   soc,  7)
  SOC_EQ(tmp, "MT9653",   "Pentonic  700",   SOC_MTK_MT9653,   soc,  7)
  SOC_EQ(tmp, "MT9689",   "Pentonic  700",   SOC_MTK_MT9689,   soc,  7) // !! Assumption only, needs confirmation
  SOC_EQ(tmp, "MT9972",   "Pentonic 1000",   SOC_MTK_MT9972,   soc,  7) // !! Assumption only, needs confirmation
  SOC_EQ(tmp, "MT9902",   "Pentonic 2000",   SOC_MTK_MT9902,   soc,  7)
  SOC_EQ(tmp, "MT9982",   "Pentonic 2000",   SOC_MTK_MT9982,   soc,  7)
  // MT XXXX //
  SOC_EQ(tmp, "MT5327",   "MT5327",          SOC_MTK_MT5327,   soc, NA)
  SOC_EQ(tmp, "MT5329",   "MT5329",          SOC_MTK_MT5329,   soc, NA)
  SOC_EQ(tmp, "MT5366",   "MT5366",          SOC_MTK_MT5366,   soc, NA)
  SOC_EQ(tmp, "MT5389",   "MT5389",          SOC_MTK_MT5389,   soc, NA)
  SOC_EQ(tmp, "MT5395",   "MT5395",          SOC_MTK_MT5395,   soc, NA)
  SOC_EQ(tmp, "MT5396",   "MT5396",          SOC_MTK_MT5396,   soc, NA)
  SOC_EQ(tmp, "MT5398",   "MT5398",          SOC_MTK_MT5398,   soc, NA)
  SOC_EQ(tmp, "MT5505",   "MT5505",          SOC_MTK_MT5505,   soc, NA)
  SOC_EQ(tmp, "MT5561",   "MT5561",          SOC_MTK_MT5561,   soc, NA)
  SOC_EQ(tmp, "MT5580",   "MT5580",          SOC_MTK_MT5580,   soc, NA)
  SOC_EQ(tmp, "MT5582",   "MT5582",          SOC_MTK_MT5582,   soc, NA)
  SOC_EQ(tmp, "MT5592",   "MT5592",          SOC_MTK_MT5592,   soc, NA)
  SOC_EQ(tmp, "MT5595",   "MT5595",          SOC_MTK_MT5595,   soc, NA)
  SOC_EQ(tmp, "MT5596",   "MT5596",          SOC_MTK_MT5596,   soc, 28) // !! Assumption only, needs confirmation
  SOC_EQ(tmp, "MT5597",   "MT5597",          SOC_MTK_MT5597,   soc, 28) // !! Assumption only, needs confirmation
  SOC_EQ(tmp, "MT5895",   "MT5895",          SOC_MTK_MT5895,   soc, 28) // Same as MT9950*
  SOC_EQ(tmp, "MT5889",   "MT5889",          SOC_MTK_MT5889,   soc, 28) // Same as MT9615 (https://www.displayspecifications.com/en/model/97272c1f)
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
  SOC_EQ(tmp, "MT9602",   "MT9602",          SOC_MTK_MT9602,   soc, 28) // Same as MT9675*
  SOC_EQ(tmp, "MT9612",   "MT9612",          SOC_MTK_MT9612,   soc, 28) // Same as MT9685*
  SOC_EQ(tmp, "MT9613",   "MT9613",          SOC_MTK_MT9613,   soc, 28) // !! Assumption only, needs confirmation
  SOC_EQ(tmp, "MT9615",   "MT9615",          SOC_MTK_MT9615,   soc, 28) // https://gadgetversus.com/processor/mediatek-mt9615-specs/
  SOC_EQ(tmp, "MT9632",   "MT9632",          SOC_MTK_MT9632,   soc, 28) // Same as MT9675*
  SOC_EQ(tmp, "MT9638",   "MT9638",          SOC_MTK_MT9638,   soc, 28) // !! Assumption only, needs confirmation
  SOC_EQ(tmp, "MT9652",   "MT9652",          SOC_MTK_MT9652,   soc, 28) // Same as MT9613*
  SOC_EQ(tmp, "MT9675",   "MT9675",          SOC_MTK_MT9675,   soc, 28) // !! Assumption only, needs confirmation
  SOC_EQ(tmp, "MT9685",   "MT9685",          SOC_MTK_MT9685,   soc, 28) // https://gadgetversus.com/processor/mediatek-mt9685-specs/
  SOC_EQ(tmp, "MT9950",   "MT9950",          SOC_MTK_MT9950,   soc, 28) // https://gadgetversus.com/processor/mediatek-mt9950-specs/
  SOC_EQ(tmp, "MT9686",   "MT9686",          SOC_MTK_MT9686,   soc, 28) // Same as MT9613*
  // (*) Many SoCs are reported with different names but they are the same chip.
  // Source: https://en.wikipedia.org/wiki/List_of_MediaTek_systems_on_chips#Digital_television_SoCs
  SOC_END
}

/*
 * Good sources:
 * https://www.geektopia.es/es/products/company/qualcomm/socs/
 *
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

  soc->vendor = SOC_VENDOR_SNAPDRAGON;

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
  SOC_EQ(tmp, "MSM8909",        "210 / 212", SOC_SNAPD_MSM8909,        soc, 28) // In the future, we can differentiate them using frequency
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
  SOC_EQ(tmp, "SDM730G",        "730G",      SOC_SNAPD_SM7150_AB,      soc,  8) // Issue #174
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
  SOC_EQ(tmp, "SM8350-AC",      "888+",      SOC_SNAPD_SM8350,         soc,  5)
  // Snapdragon Gen 4 //
  SOC_EQ(tmp, "SM4375",         "4 Gen 1",   SOC_SNAPD_SM4375,         soc,  6)
  SOC_EQ(tmp, "SM4450",         "4 Gen 2",   SOC_SNAPD_SM4450,         soc,  4)
  SOC_EQ(tmp, "SM4635",         "4s Gen 2",  SOC_SNAPD_SM4635,         soc,  4)
  // Snapdragon Gen 6 //
  SOC_EQ(tmp, "SM6375-AC",      "6s Gen 3",  SOC_SNAPD_SM6375_AC,      soc,  6)
  SOC_EQ(tmp, "SM6450",         "6 Gen 1",   SOC_SNAPD_SM6450,         soc,  4)
  // Snapdragon Gen 7 //
  SOC_EQ(tmp, "SM7435-AB",      "7s Gen 2",  SOC_SNAPD_SM7435_AB,      soc,  4)
  SOC_EQ(tmp, "SM7450",         "7 Gen 1",   SOC_SNAPD_SM7450,         soc,  4)
  SOC_EQ(tmp, "SM7475",         "7+ Gen 2",  SOC_SNAPD_SM7475,         soc,  4)
  SOC_EQ(tmp, "SM7550-AB",      "7 Gen 3",   SOC_SNAPD_SM7550_AB,      soc,  4)
  SOC_EQ(tmp, "SM7675-AB",      "7+ Gen 3",  SOC_SNAPD_SM7675_AB,      soc,  4)
  // Snapdragon Gen 8 //
  SOC_EQ(tmp, "SM8450",         "8 Gen 1",   SOC_SNAPD_SM8450,         soc,  4)
  SOC_EQ(tmp, "SM8475",         "8+ Gen 1",  SOC_SNAPD_SM8475,         soc,  4)
  SOC_EQ(tmp, "SM8550-AB",      "8 Gen 2",   SOC_SNAPD_SM8550_AB,      soc,  4)
  SOC_EQ(tmp, "SM8635",         "8s Gen 3",  SOC_SNAPD_SM8635,         soc,  4)
  SOC_EQ(tmp, "SM8650-AB",      "8 Gen 3",   SOC_SNAPD_SM8650_AB,      soc,  4)
  SOC_END
}

// https://linux-sunxi.org/Allwinner_SoC_Family
bool match_allwinner(char* soc_name, struct system_on_chip* soc) {
  char* tmp;

  if((tmp = strstr(soc_name, "sun")) == NULL)
    return false;

  soc->vendor = SOC_VENDOR_ALLWINNER;

  SOC_START
  // SoCs we can detect just with with the name
  SOC_EQ(tmp, "sun4i", "A10",   SOC_ALLWINNER_A10,  soc, 55)
  SOC_EQ(tmp, "sun6i", "A31",   SOC_ALLWINNER_A31,  soc, 40)
  SOC_EQ(tmp, "sun7i", "A20",   SOC_ALLWINNER_A20,  soc, 40)
  else {
    // sun5i/sun8i/sun9i/sun50i will fall here
    // We need SID to actually distingish between the exact model
    int filelen;
    char* sid_nvmem = read_file(_PATH_SUNXI_NVMEM, &filelen);
    if(sid_nvmem == NULL) {
      printWarn("read_file: %s: %s", _PATH_SUNXI_NVMEM, strerror(errno));
      return false;
    }
    uint32_t sid = get_sid_from_nvmem(sid_nvmem);
    return get_sunxisoc_from_sid(soc, soc_name, sid);
  }
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

  // New Snapdragon SoCs codenames
  // https://github.com/sm8450-mainline/fdt?tab=readme-ov-file#chipsets
  // https://github.com/Dr-Noob/cpufetch/issues/253
  if (strcmp(soc_name, "cape") == 0) {
    fill_soc(soc, "8+ Gen 1", SOC_SNAPD_SM8475, 4);
    return true;
  }

  if(strcmp(soc_name, "taro") == 0) {
    fill_soc(soc, "8 Gen 1", SOC_SNAPD_SM8450, 4);
    return true;
  }

  if(strcmp(soc_name, "ukee") == 0) {
    fill_soc(soc, "7+ Gen 2", SOC_SNAPD_SM7475, 4);
    return true;
  }

  if(strcmp(soc_name, "diwali") == 0) {
    fill_soc(soc, "7 Gen 1", SOC_SNAPD_SM7450, 4);
    return true;
  }

  // parrot can be either SM7435 or SM6450, we need more data
  // to distingish between those two

  if(strcmp(soc_name, "ravelin") == 0) {
    fill_soc(soc, "4 Gen 2", SOC_SNAPD_SM4450, 4);
    return true;
  }

  // Google Pixel 6
  // https://github.com/Dr-Noob/cpufetch/issues/134
  if(strcmp(soc_name, "oriole") == 0) {
    fill_soc(soc, "Tensor", SOC_GOOGLE_TENSOR, 5);
    return true;
  }

  // Google Pixel 8
  // https://github.com/Dr-Noob/cpufetch/issues/198
  if(strcmp(soc_name, "husky") == 0 ||
     strcmp(soc_name, "zuma") == 0) {
    fill_soc(soc, "Tensor G3", SOC_GOOGLE_TENSOR_G3, 4);
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

  if(match_allwinner(raw_name, soc))
    return soc;

  if(match_google(raw_name, soc))
    return soc;

  match_broadcom(raw_name, soc);
  return soc;
}

#ifdef __ANDROID__
#include <sys/system_properties.h>

static inline int android_property_get(const char* key, char* value) {
  return __system_property_get(key, value);
}

void try_parse_soc_from_string(struct system_on_chip* soc, int soc_len, char* soc_str) {
  soc->raw_name = emalloc(sizeof(char) * (soc_len + 1));
  strncpy(soc->raw_name, soc_str, soc_len + 1);
  soc->raw_name[soc_len] = '\0';
  soc->vendor = SOC_VENDOR_UNKNOWN;
  parse_soc_from_string(soc);
}

struct system_on_chip* guess_soc_from_android(struct system_on_chip* soc) {
  char tmp[100];
  int property_len = 0;

  property_len = android_property_get(ANDROID_PROP_MTK_PLATFORM, (char *) &tmp);
  if(property_len > 0) {
    try_parse_soc_from_string(soc, property_len, tmp);
    if(soc->vendor == SOC_VENDOR_UNKNOWN) printWarn("SoC detection failed using Android property %s: %s", PROP_MTK_PLATFORM, tmp);
    else return soc;
  }

  // https://github.com/Dr-Noob/cpufetch/issues/253
  // ro.soc.model might be more reliable than ro.product.board or
  // ro.board.platform, so try with it first
  property_len = android_property_get(PROP_SOC_MODEL, (char *) &tmp);
  if(property_len > 0) {
    try_parse_soc_from_string(soc, property_len, tmp);
    if(soc->vendor == SOC_VENDOR_UNKNOWN) printWarn("SoC detection failed using Android property %s: %s", PROP_SOC_MODEL, tmp);
    else return soc;
  }

  property_len = android_property_get(PROP_PRODUCT_BOARD, (char *) &tmp);
  if(property_len > 0) {
    try_parse_soc_from_string(soc, property_len, tmp);
    if(soc->vendor == SOC_VENDOR_UNKNOWN) printWarn("SoC detection failed using Android property %s: %s", PROP_PRODUCT_BOARD, tmp);
    else return soc;
  }

  property_len = android_property_get(PROP_BOARD_PLATFORM, (char *) &tmp);
  if(property_len > 0) {
    try_parse_soc_from_string(soc, property_len, tmp);
    if(soc->vendor == SOC_VENDOR_UNKNOWN) printWarn("SoC detection failed using Android property %s: %s", PROP_BOARD_PLATFORM, tmp);
    else return soc;
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

char* get_rk_efuse(void) {
  int filelen;
  char* rk_soc = read_file(_PATH_RK_EFUSE0, &filelen);
  if(rk_soc == NULL) {
    printWarn("read_file: %s: %s", _PATH_RK_EFUSE0, strerror(errno));
    rk_soc = read_file(_PATH_RK_OTP0, &filelen);
    if(rk_soc == NULL) {
      printWarn("read_file: %s: %s", _PATH_RK_OTP0, strerror(errno));
    }
  }
  return rk_soc;
}

bool get_rk_soc_from_efuse(struct system_on_chip* soc, char* efuse) {
  typedef struct {
    uint16_t rk_soc;
    struct system_on_chip soc;
  } rkToSoC;

  uint16_t rk_soc = efuse[2]*256 + efuse[3];

  // https://wikimovel.com/index.php/Rockchip
  rkToSoC socFromRK[] = {
    // TODO: Add RK2XXX
    // RK3XXX
    // Reverse order
    {0x2388, {SOC_ROCKCHIP_3288,  SOC_VENDOR_ROCKCHIP, 28, "RK3288",  NULL} },
    {0x2392, {SOC_ROCKCHIP_3229,  SOC_VENDOR_ROCKCHIP, 28, "RK3229",  NULL} }, // https://gadgetversus.com/processor/rockchip-rk3229-vs-rockchip-rk3128/
    {0x3380, {SOC_ROCKCHIP_3308,  SOC_VENDOR_ROCKCHIP, 28, "RK3308",  NULL} }, // https://en.t-firefly.com/product/rocrk3308cc?theme=pc
    {0x3381, {SOC_ROCKCHIP_3318,  SOC_VENDOR_ROCKCHIP, 28, "RK3318",  NULL} },
    {0x3362, {SOC_ROCKCHIP_3326,  SOC_VENDOR_ROCKCHIP, 28, "RK3326",  NULL} },
    {0x3382, {SOC_ROCKCHIP_3328,  SOC_VENDOR_ROCKCHIP, 28, "RK3328",  NULL} },
    {0x3386, {SOC_ROCKCHIP_3368,  SOC_VENDOR_ROCKCHIP, 28, "RK3368",  NULL} },
    {0x3399, {SOC_ROCKCHIP_3399,  SOC_VENDOR_ROCKCHIP, 28, "RK3399",  NULL} },
    // Normal Order (https://github.com/Dr-Noob/cpufetch/issues/209)
    {0x3528, {SOC_ROCKCHIP_3528,  SOC_VENDOR_ROCKCHIP, 28, "RK3528",  NULL} },
    {0x3562, {SOC_ROCKCHIP_3562,  SOC_VENDOR_ROCKCHIP, 22, "RK3562",  NULL} },
    {0x3566, {SOC_ROCKCHIP_3566,  SOC_VENDOR_ROCKCHIP, 22, "RK3566",  NULL} },
    {0x3568, {SOC_ROCKCHIP_3568,  SOC_VENDOR_ROCKCHIP, 22, "RK3568",  NULL} },
    {0x3588, {SOC_ROCKCHIP_3588,  SOC_VENDOR_ROCKCHIP,  8, "RK3588",  NULL} }, // No known way to distingish between S version: https://github.com/Dr-Noob/cpufetch/issues/188,209
    // Unknown
    {0x0000, {UNKNOWN,            SOC_VENDOR_UNKNOWN,   -1,       "", NULL} }
  };

  int index = 0;
  while(socFromRK[index].rk_soc != 0x0) {
    if(socFromRK[index].rk_soc == rk_soc) {
      fill_soc(soc, socFromRK[index].soc.name, socFromRK[index].soc.model, socFromRK[index].soc.process);
      return true;
    }
    index++;
  }

  printErr("RK SoC was found but it does not match any known SoCs: 0x%04x", rk_soc);
  return false;
}

struct system_on_chip* guess_soc_from_nvmem(struct system_on_chip* soc) {
  // This method is only valid for Rockchip SoCs
  char* rk_soc = get_rk_efuse();
  if(rk_soc != NULL) {
    if(rk_soc[0] == 0x52 && rk_soc[1] == 0x4b) {
      get_rk_soc_from_efuse(soc, rk_soc);
      return soc;
    }
    else {
      printWarn("guess_soc_from_nvmem: efuse found, but contains unexpected header: 0x%x 0x%x", rk_soc[0], rk_soc[1]);
    }
  }
  return soc;
}

struct system_on_chip* guess_soc_from_uarch(struct system_on_chip* soc, struct cpuInfo* cpu) {
  // Currently we only support CPUs with only one uarch (in other words, one socket)
  struct uarch* arch = cpu->arch;
  if (arch == NULL) {
    printWarn("guess_soc_from_uarch: uarch is NULL");
    return soc;
  }

  typedef struct {
    MICROARCH u;
    struct system_on_chip soc;
  } uarchToSoC;

  uarchToSoC socFromUarch[] = {
    {UARCH_TAISHAN_V110, {SOC_KUNPENG_920,  SOC_VENDOR_KUNPENG,  7, "920", NULL} },
    {UARCH_TAISHAN_V200, {SOC_KUNPENG_930,  SOC_VENDOR_KUNPENG,  7, "930", NULL} }, // manufacturing process is not well-known
    {UARCH_UNKNOWN,      {UNKNOWN,          SOC_VENDOR_UNKNOWN, -1,    "", NULL} }
  };

  int index = 0;
  while(socFromUarch[index].u != UARCH_UNKNOWN) {
    if(socFromUarch[index].u == get_uarch(arch)) {
      fill_soc(soc, socFromUarch[index].soc.name, socFromUarch[index].soc.model, socFromUarch[index].soc.process);
      return soc;
    }
    index++;
  }

  printWarn("guess_soc_from_uarch: No uarch matched the list");
  return soc;
}

// Return the dt string without the NULL characters.
char* get_dt_str(char* dt, int filelen) {
  char* dt_without_null = (char *) malloc(sizeof(char) * filelen);
  memcpy(dt_without_null, dt, filelen);

  for (int i=0; i < filelen-1; i++) {
    if (dt_without_null[i] == '\0')
      dt_without_null[i] = ',';
  }
  return dt_without_null;
}

bool match_dt(struct system_on_chip* soc, char* dt, int filelen, char* expected_name, char* soc_name, SOC soc_model, int32_t process) {
  // The /proc/device-tree/compatible file (passed by dt) uses NULL
  // to separate the strings, so we need to make an special case here
  // and iterate over the NULL characters, thus iterating over each
  // individual compatible strings.

  if (strstr(dt, expected_name) != NULL) {
    fill_soc(soc, soc_name, soc_model, process);
    return true;
  }

  char *compatible = dt;
  char *end_of_dt = dt + filelen;

  while ((compatible = strchr(compatible, '\0')) != end_of_dt) {
    compatible++;
    if (strstr(compatible, expected_name) != NULL) {
      fill_soc(soc, soc_name, soc_model, process);
      return true;
    }
  }

  return false;
}

#define DT_START if (false) {}
#define DT_EQ(dt, filelen, soc, expected_name, soc_name, soc_model, process) \
   else if (match_dt(soc, dt, filelen, expected_name, soc_name, soc_model, process)) return soc;
#define DT_END(dt, filelen) else { printWarn("guess_soc_from_devtree: No match found for '%s'", get_dt_str(dt, filelen)); return soc; }

// TODO: Move this to doc
// The number of fields seems non-standard, so for now it seems wiser
// to just get the entire string with all fields and just look for the
// substring.
// TODO: Implement this by going trough NULL-separated fields rather than
// using strstr.
struct system_on_chip* guess_soc_from_devtree(struct system_on_chip* soc) {
  int len;
  char* dt = get_devtree_compatible(&len);
  if (dt == NULL) {
    return soc;
  }

  // The following are internal codenames of Asahi Linux
  // https://github.com/AsahiLinux/docs/wiki/Codenames
  DT_START
  DT_EQ(dt, len, soc, "apple,t8103", "M1",       SOC_APPLE_M1,       5)
  DT_EQ(dt, len, soc, "apple,t6000", "M1 Pro",   SOC_APPLE_M1_PRO,   5)
  DT_EQ(dt, len, soc, "apple,t6001", "M1 Max",   SOC_APPLE_M1_MAX,   5)
  DT_EQ(dt, len, soc, "apple,t6002", "M1 Ultra", SOC_APPLE_M1_ULTRA, 5)
  DT_EQ(dt, len, soc, "apple,t8112", "M2",       SOC_APPLE_M2,       5)
  DT_EQ(dt, len, soc, "apple,t6020", "M2 Pro",   SOC_APPLE_M2_PRO,   5)
  DT_EQ(dt, len, soc, "apple,t6021", "M2 Max",   SOC_APPLE_M2_MAX,   5)
  DT_EQ(dt, len, soc, "apple,t6022", "M2 Ultra", SOC_APPLE_M2_ULTRA, 5)
  DT_EQ(dt, len, soc, "apple,t8122", "M3",       SOC_APPLE_M3,       3)
  DT_EQ(dt, len, soc, "apple,t6030", "M3 Pro",   SOC_APPLE_M3_PRO,   3)
  DT_EQ(dt, len, soc, "apple,t6031", "M3 Max",   SOC_APPLE_M3_MAX,   3)
  DT_EQ(dt, len, soc, "apple,t6034", "M3 Max",   SOC_APPLE_M3_MAX,   3)
  DT_END(dt, len)
}

struct system_on_chip* guess_soc_from_pci(struct system_on_chip* soc, struct cpuInfo* cpu) {
  struct pci_devices * pci = get_pci_devices();
  if (pci == NULL) {
    printWarn("guess_soc_from_pci: Unable to find suitable PCI devices");
    return soc;
  }

  typedef struct {
    uint16_t vendor_id;
    uint16_t device_id;
    struct system_on_chip soc;
  } pciToSoC;

  pciToSoC socFromPCI[] = {
    {PCI_VENDOR_NVIDIA, PCI_DEVICE_TEGRA_X1, {SOC_TEGRA_X1,     SOC_VENDOR_NVIDIA,  20, "Tegra X1", NULL} },
    // {PCI_VENDOR_NVIDIA, PCI_DEVICE_GH_200,{SOC_GH_200,       SOC_VENDOR_NVIDIA,   ?, "Grace Hopper", NULL} },
    {PCI_VENDOR_AMPERE, PCI_DEVICE_ALTRA,    {SOC_AMPERE_ALTRA, SOC_VENDOR_AMPERE,   7, "Altra",    NULL} }, // https://www.anandtech.com/show/15575/amperes-altra-80-core-n1-soc-for-hyperscalers-against-rome-and-xeon
    {0x0000,            0x0000,              {UNKNOWN,          SOC_VENDOR_UNKNOWN, -1,      "",    NULL} }
  };

  int index = 0;
  while (socFromPCI[index].vendor_id != 0x0) {
    for (int i=0; i < pci->num_devices; i++) {
      struct pci_device * dev = pci->devices[i];

      if (socFromPCI[index].vendor_id == dev->vendor_id &&
          socFromPCI[index].device_id == dev->device_id) {
        fill_soc(soc, socFromPCI[index].soc.name, socFromPCI[index].soc.model, socFromPCI[index].soc.process);
        return soc;
      }
    }
    index++;
  }

  printWarn("guess_soc_from_pci: No PCI device matched the list");
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

/*
 * https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#raspberry-pi-revision-codes:
 * NOTE: As of the 4.9 kernel, all Raspberry Pi computers report BCM2835, even those with BCM2836,
 * BCM2837 and BCM2711 processors. You should not use this string to detect the processor. Decode the
 * revision code using the information below.
 * https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#new-style-revision-codes-in-use
 */
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
  if(pppp < 0) {
    printBug("[RPi] Found invalid RPi PPPP code: %s", pppp);
    return soc;
  }

  if(pppp > arr_size-1) {
    printBug("[RPi] Found invalid RPi PPPP code: %d (max is %d)", pppp, arr_size-1);
    return soc;
  }

  char* soc_raw_name = soc_rpi_string[pppp];
  match_broadcom(soc_raw_name, soc);

  return soc;
}

#if defined(__APPLE__) || defined(__MACH__)
struct system_on_chip* guess_soc_apple(struct system_on_chip* soc) {
  uint32_t cpu_family = get_sys_info_by_name("hw.cpufamily");
  uint32_t cpu_subfamily = get_sys_info_by_name("hw.cpusubfamily");

  if(cpu_family == CPUFAMILY_ARM_FIRESTORM_ICESTORM) {
    // Check M1 version
    if(cpu_subfamily == CPUSUBFAMILY_ARM_HG) {
      fill_soc(soc, "M1", SOC_APPLE_M1, 5);
    }
    else if(cpu_subfamily == CPUSUBFAMILY_ARM_HS) {
      fill_soc(soc, "M1 Pro", SOC_APPLE_M1_PRO, 5);
    }
    else if(cpu_subfamily == CPUSUBFAMILY_ARM_HC_HD) {
      // Could be M1 Max or M1 Ultra (2x M1 Max)
      uint32_t physicalcpu = get_sys_info_by_name("hw.physicalcpu");
      if(physicalcpu == 20) {
        fill_soc(soc, "M1 Ultra", SOC_APPLE_M1_ULTRA, 5);
      }
      else if(physicalcpu == 10) {
        fill_soc(soc, "M1 Max", SOC_APPLE_M1_MAX, 5);
      }
      else {
        printBug("Found invalid physical cpu number: %d", physicalcpu);
        soc->vendor = SOC_VENDOR_UNKNOWN;
      }
    }
    else {
      printBugCheckRelease("Found invalid cpu_subfamily: 0x%.8X", cpu_subfamily);
      soc->vendor = SOC_VENDOR_UNKNOWN;
    }
  }
  else if(cpu_family == CPUFAMILY_ARM_AVALANCHE_BLIZZARD) {
    // Check M2 version
    if(cpu_subfamily == CPUSUBFAMILY_ARM_HG) {
      fill_soc(soc, "M2", SOC_APPLE_M2, 5);
    }
    else if(cpu_subfamily == CPUSUBFAMILY_ARM_HS) {
      fill_soc(soc, "M2 Pro", SOC_APPLE_M2_PRO, 5);
    }
    else if(cpu_subfamily == CPUSUBFAMILY_ARM_HC_HD) {
      // Could be M2 Max or M2 Ultra (2x M1 Max)
      uint32_t physicalcpu = get_sys_info_by_name("hw.physicalcpu");
      if(physicalcpu == 24) {
        fill_soc(soc, "M2 Ultra", SOC_APPLE_M2_ULTRA, 5);
      }
      else if(physicalcpu == 12) {
        fill_soc(soc, "M2 Max", SOC_APPLE_M2_MAX, 5);
      }
      else {
        printBug("Found invalid physical cpu number: %d", physicalcpu);
        soc->vendor = SOC_VENDOR_UNKNOWN;
      }
    }
    else {
      printBugCheckRelease("Found invalid cpu_subfamily: 0x%.8X", cpu_subfamily);
      soc->vendor = SOC_VENDOR_UNKNOWN;
    }
  }
  else if(cpu_family == CPUFAMILY_ARM_EVEREST_SAWTOOTH ||
          cpu_family == CPUFAMILY_ARM_EVEREST_SAWTOOTH_PRO ||
          cpu_family == CPUFAMILY_ARM_EVEREST_SAWTOOTH_MAX) {
    // Check M3 version
    if(cpu_family == CPUFAMILY_ARM_EVEREST_SAWTOOTH) {
      fill_soc(soc, "M3", SOC_APPLE_M3, 3);
    }
    else if(cpu_family == CPUFAMILY_ARM_EVEREST_SAWTOOTH_PRO) {
      fill_soc(soc, "M3 Pro", SOC_APPLE_M3_PRO, 3);
    }
    else if(cpu_family == CPUFAMILY_ARM_EVEREST_SAWTOOTH_MAX) {
      fill_soc(soc, "M3 Max", SOC_APPLE_M3_MAX, 3);
    }
    else {
      printBugCheckRelease("Found invalid cpu_family: 0x%.8X", cpu_family);
      soc->vendor = SOC_VENDOR_UNKNOWN;
    }
  }
  else {
    printBugCheckRelease("Found invalid cpu_family: 0x%.8X", cpu_family);
    soc->vendor = SOC_VENDOR_UNKNOWN;
  }
  return soc;
}
#endif

struct system_on_chip* get_soc(struct cpuInfo* cpu) {
  struct system_on_chip* soc = emalloc(sizeof(struct system_on_chip));
  soc->raw_name = NULL;
  soc->vendor = SOC_VENDOR_UNKNOWN;
  soc->model = SOC_MODEL_UNKNOWN;
  soc->process = UNKNOWN;

#ifdef __linux__
  bool isRPi = is_raspberry_pi();
  if(isRPi) {
    soc = guess_soc_raspbery_pi(soc);
    if(soc->vendor == SOC_VENDOR_UNKNOWN) {
      printErr("[RPi] SoC detection failed using revision code, falling back to cpuinfo detection");
    }
    else {
      return soc;
    }
  }

  soc = guess_soc_from_cpuinfo(soc);
  if(soc->vendor == SOC_VENDOR_UNKNOWN) {
    if(soc->raw_name != NULL) {
      printWarn("SoC detection failed using /proc/cpuinfo: Found '%s' string", soc->raw_name);
    }
    else {
      printWarn("SoC detection failed using /proc/cpuinfo: No string found");
    }
#ifdef __ANDROID__
    soc = guess_soc_from_android(soc);
    if(soc->raw_name == NULL) {
      printWarn("SoC detection failed using Android: No string found");
    }
    else if(soc->vendor == SOC_VENDOR_UNKNOWN) {
      printWarn("SoC detection failed using Android: Found '%s' string", soc->raw_name);
    }
#endif // ifdef __ANDROID__
    // If previous steps failed, try with the device tree
    if (soc->vendor == SOC_VENDOR_UNKNOWN) {
      soc = guess_soc_from_devtree(soc);
    }
    // If previous steps failed, try with nvmem
    if(soc->vendor == SOC_VENDOR_UNKNOWN) {
      soc = guess_soc_from_nvmem(soc);
    }
    // If previous steps failed, try infering it from the microarchitecture
    if(soc->vendor == SOC_VENDOR_UNKNOWN) {
      soc = guess_soc_from_uarch(soc, cpu);
    }
    // If previous steps failed, try infering it from the pci device id
    if(soc->vendor == SOC_VENDOR_UNKNOWN) {
      soc = guess_soc_from_pci(soc, cpu);
    }
  }
#elif defined __APPLE__ || __MACH__
  soc = guess_soc_apple(soc);
  if(soc->vendor == SOC_VENDOR_UNKNOWN) {
    printWarn("SoC detection failed using cpu_subfamily");
  }
  else {
    return soc;
  }
#endif // ifdef __linux__

  if(soc->model == SOC_MODEL_UNKNOWN) {
    // raw_name might not be NULL, but if we were unable to find
    // the exact SoC, just print "Unkwnown"
    soc->raw_name = emalloc(sizeof(char) * (strlen(STRING_UNKNOWN)+1));
    snprintf(soc->raw_name, strlen(STRING_UNKNOWN)+1, STRING_UNKNOWN);
  }

  return soc;
}
