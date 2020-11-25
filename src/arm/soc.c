#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "soc.h"
#include "udev.h"
#include "../common/global.h"

#define min(a,b) (((a)<(b))?(a):(b))
#define STRING_UNKNOWN    "Unknown"

static char* soc_trademark_string[] = {
  [SOC_SNAPDRAGON] = "Snapdragon",
  [SOC_MEDIATEK]   = "MediaTek",
};

bool match_soc(struct system_on_chip* soc, char* raw_name, char* expected_name, char* soc_name, SOC soc_vendor, int32_t process) {
  int len = min(strlen(raw_name), strlen(expected_name));
  if(strncmp(raw_name, expected_name, len) != 0) return false;
  
  soc->soc_vendor = soc_vendor;
  soc->process = process;
  len = strlen(soc_name) + strlen(soc_trademark_string[soc->soc_vendor]) + 1;
  soc->soc_name = malloc(sizeof(char) * len);
  memset(soc->soc_name, 0, sizeof(char) * len);
  sprintf(soc->soc_name, "%s %s", soc_trademark_string[soc->soc_vendor], soc_name);
  
  return true;
}

#define SOC_START if (false) {}
#define CHECK_SOC(raw_name, expected_name, soc_name, soc_vendor, len, soc, process) \
   else if (match_soc(soc, raw_name, expected_name, soc_name, soc_vendor, process)) return true;
#define SOC_END else { return false; }

bool match_qualcomm(char* soc_name, struct system_on_chip* soc) {
  char* tmp;

  if((tmp = strstr(soc_name, "MSM")) != NULL);
  else if((tmp = strstr(soc_name, "SDM")) != NULL);
  else if((tmp = strstr(soc_name, "APQ")) != NULL);
  else if((tmp = strstr(soc_name, "SM")) != NULL);    
  else if((tmp = strstr(soc_name, "QM")) != NULL);      
  else return false;
  
  SOC_START
  // Snapdragon S1 //
  CHECK_SOC(tmp, "MSM7627A",       "S1",        SOC_SNAPDRAGON, len, soc, 65) 
  CHECK_SOC(tmp, "MSM7225",        "S1",        SOC_SNAPDRAGON, len, soc, 65) 
  CHECK_SOC(tmp, "MSM7625",        "S1",        SOC_SNAPDRAGON, len, soc, 65) 
  CHECK_SOC(tmp, "MSM7625A",       "S1",        SOC_SNAPDRAGON, len, soc, 45) 
  CHECK_SOC(tmp, "MSM7627",        "S1",        SOC_SNAPDRAGON, len, soc, 65) 
  CHECK_SOC(tmp, "MSM7227A",       "S1",        SOC_SNAPDRAGON, len, soc, 45) 
  CHECK_SOC(tmp, "QSD8250",        "S1",        SOC_SNAPDRAGON, len, soc, 65) 
  CHECK_SOC(tmp, "MSM7227",        "S1",        SOC_SNAPDRAGON, len, soc, 65) 
  CHECK_SOC(tmp, "MSM7225A",       "S1",        SOC_SNAPDRAGON, len, soc, 45) 
  // Snapdragon S2 //
  CHECK_SOC(tmp, "MSM7230",        "S2",        SOC_SNAPDRAGON, len, soc, 45) 
  CHECK_SOC(tmp, "MSM8655T",       "S2",        SOC_SNAPDRAGON, len, soc, 45) 
  CHECK_SOC(tmp, "MSM8255",        "S2",        SOC_SNAPDRAGON, len, soc, 45) 
  CHECK_SOC(tmp, "MSM8255T",       "S2",        SOC_SNAPDRAGON, len, soc, 45) 
  CHECK_SOC(tmp, "MSM8655",        "S2",        SOC_SNAPDRAGON, len, soc, 45) 
  CHECK_SOC(tmp, "APQ8055",        "S2",        SOC_SNAPDRAGON, len, soc, 45) 
  // Snapdragon S3 //
  CHECK_SOC(tmp, "MSM8260",        "S3",        SOC_SNAPDRAGON, len, soc, 45) 
  CHECK_SOC(tmp, "MSM8660",        "S3",        SOC_SNAPDRAGON, len, soc, 45) 
  CHECK_SOC(tmp, "APQ8060",        "S3",        SOC_SNAPDRAGON, len, soc, 45) 
  // Snapdragon S4 //
  CHECK_SOC(tmp, "MSM8225",        "S4 Play",   SOC_SNAPDRAGON, len, soc, 45) 
  CHECK_SOC(tmp, "MSM8225Q",       "S4 Play",   SOC_SNAPDRAGON, len, soc, 45) 
  CHECK_SOC(tmp, "MSM8227",        "S4 Plus",   SOC_SNAPDRAGON, len, soc, 28) 
  CHECK_SOC(tmp, "MSM8960",        "S4 Plus",   SOC_SNAPDRAGON, len, soc, 28) 
  CHECK_SOC(tmp, "MSM8930",        "S4 Plus",   SOC_SNAPDRAGON, len, soc, 28) 
  CHECK_SOC(tmp, "MSM8260A",       "S4 Plus",   SOC_SNAPDRAGON, len, soc, 28) 
  CHECK_SOC(tmp, "MSM8230",        "S4 Plus",   SOC_SNAPDRAGON, len, soc, 28) 
  CHECK_SOC(tmp, "APQ8064",        "S4 Pro",    SOC_SNAPDRAGON, len, soc, 28) 
  // Snapdragon 2XX //
  CHECK_SOC(tmp, "MSM8212",        "200",       SOC_SNAPDRAGON, len, soc, 45) 
  CHECK_SOC(tmp, "MSM8625Q",       "200",       SOC_SNAPDRAGON, len, soc, 45)
  CHECK_SOC(tmp, "MSM8210",        "200",       SOC_SNAPDRAGON, len, soc, 45)
  CHECK_SOC(tmp, "MSM8225Q",       "200",       SOC_SNAPDRAGON, len, soc, 45)
  CHECK_SOC(tmp, "MSM8208",        "208",       SOC_SNAPDRAGON, len, soc, 28)
  CHECK_SOC(tmp, "MSM8909",        "210",       SOC_SNAPDRAGON, len, soc, 28)
  CHECK_SOC(tmp, "APQ8009",        "212",       SOC_SNAPDRAGON, len, soc, 28)
  CHECK_SOC(tmp, "QM215",          "215",       SOC_SNAPDRAGON, len, soc, 28)
  // Snapdragon 4XX //
  CHECK_SOC(tmp, "MSM8226",        "400",       SOC_SNAPDRAGON, len, soc, 28)
  CHECK_SOC(tmp, "MSM8926",        "400",       SOC_SNAPDRAGON, len, soc, 28)
  CHECK_SOC(tmp, "MSM8930",        "400",       SOC_SNAPDRAGON, len, soc, 28)
  CHECK_SOC(tmp, "MSM8928",        "400",       SOC_SNAPDRAGON, len, soc, 28)
  CHECK_SOC(tmp, "MSM8230AB",      "400",       SOC_SNAPDRAGON, len, soc, 28)
  CHECK_SOC(tmp, "MSM8228",        "400",       SOC_SNAPDRAGON, len, soc, 28)
  CHECK_SOC(tmp, "MSM8930AA",      "400",       SOC_SNAPDRAGON, len, soc, 28)
  CHECK_SOC(tmp, "MSM8916T",       "412",       SOC_SNAPDRAGON, len, soc, 28)
  CHECK_SOC(tmp, "MSM8916",        "410",       SOC_SNAPDRAGON, len, soc, 28)  
  CHECK_SOC(tmp, "MSM8929",        "415",       SOC_SNAPDRAGON, len, soc, 28)
  CHECK_SOC(tmp, "MSM8917",        "425",       SOC_SNAPDRAGON, len, soc, 28)
  CHECK_SOC(tmp, "MSM8920",        "427",       SOC_SNAPDRAGON, len, soc, 28)
  CHECK_SOC(tmp, "SDM429",         "429",       SOC_SNAPDRAGON, len, soc, 12)
  CHECK_SOC(tmp, "MSM8937",        "430",       SOC_SNAPDRAGON, len, soc, 28)
  CHECK_SOC(tmp, "MSM8940",        "435",       SOC_SNAPDRAGON, len, soc, 28)
  CHECK_SOC(tmp, "SDM439",         "439",       SOC_SNAPDRAGON, len, soc, 12)
  CHECK_SOC(tmp, "SDM450",         "450",       SOC_SNAPDRAGON, len, soc, 14)
  CHECK_SOC(tmp, "SM4250",         "460",       SOC_SNAPDRAGON, len, soc, 11)
  // Snapdragon 6XX //
  CHECK_SOC(tmp, "APQ8064T",       "600",       SOC_SNAPDRAGON, len, soc, 28)
  CHECK_SOC(tmp, "MSM8936",        "610",       SOC_SNAPDRAGON, len, soc, 28)
  CHECK_SOC(tmp, "MSM8939",        "615 / 616", SOC_SNAPDRAGON, len, soc, 28)
  CHECK_SOC(tmp, "MSM8952",        "617",       SOC_SNAPDRAGON, len, soc, 28)
  CHECK_SOC(tmp, "MSM8953",        "625",       SOC_SNAPDRAGON, len, soc, 14)
  CHECK_SOC(tmp, "MSM8953 Pro",    "626",       SOC_SNAPDRAGON, len, soc, 14)
  CHECK_SOC(tmp, "SDM630",         "630",       SOC_SNAPDRAGON, len, soc, 14)
  CHECK_SOC(tmp, "SDM632",         "632",       SOC_SNAPDRAGON, len, soc, 12)
  CHECK_SOC(tmp, "SDM636",         "636",       SOC_SNAPDRAGON, len, soc, 14)
  CHECK_SOC(tmp, "MSM8956",        "650",       SOC_SNAPDRAGON, len, soc, 28)
  CHECK_SOC(tmp, "MSM8976",        "652",       SOC_SNAPDRAGON, len, soc, 28)
  CHECK_SOC(tmp, "MSM8976 Pro",    "653",       SOC_SNAPDRAGON, len, soc, 28)
  CHECK_SOC(tmp, "SDM660",         "660",       SOC_SNAPDRAGON, len, soc, 14)
  CHECK_SOC(tmp, "SM6115",         "662",       SOC_SNAPDRAGON, len, soc, 11)
  CHECK_SOC(tmp, "SM6125",         "665",       SOC_SNAPDRAGON, len, soc, 11)
  CHECK_SOC(tmp, "SDM670",         "670",       SOC_SNAPDRAGON, len, soc, 10)
  CHECK_SOC(tmp, "SDM675",         "675",       SOC_SNAPDRAGON, len, soc, 11)
  CHECK_SOC(tmp, "SDM690",         "690",       SOC_SNAPDRAGON, len, soc,  8)
  // Snapdragon 7XX //
  //CHECK_SOC(tmp, "?",            "710",       SOC_SNAPDRAGON, len, soc, 10)
  //CHECK_SOC(tmp, "?",            "712",       SOC_SNAPDRAGON, len, soc, 10)
  CHECK_SOC(tmp, "SM7125",         "720G",      SOC_SNAPDRAGON, len, soc,  8)
  CHECK_SOC(tmp, "SM7150-AA",      "730",       SOC_SNAPDRAGON, len, soc,  8)
  CHECK_SOC(tmp, "SM7150-AB",      "730G",      SOC_SNAPDRAGON, len, soc,  8)
  CHECK_SOC(tmp, "SM7150-AC",      "732G",      SOC_SNAPDRAGON, len, soc,  8)
  CHECK_SOC(tmp, "SM7225",         "750G",      SOC_SNAPDRAGON, len, soc,  8)
  CHECK_SOC(tmp, "SM7250-AA",      "765",       SOC_SNAPDRAGON, len, soc,  7)
  CHECK_SOC(tmp, "SM7250-AB",      "765G",      SOC_SNAPDRAGON, len, soc,  7)
  CHECK_SOC(tmp, "SM7250-AC",      "768G",      SOC_SNAPDRAGON, len, soc,  7)
  // Snapdragon 8XX //
  CHECK_SOC(tmp, "MSM8974AA",      "800",       SOC_SNAPDRAGON, len, soc, 28)
  CHECK_SOC(tmp, "MSM8974AB",      "800",       SOC_SNAPDRAGON, len, soc, 28)
  CHECK_SOC(tmp, "MSM8974AC",      "800",       SOC_SNAPDRAGON, len, soc, 28)
  CHECK_SOC(tmp, "MSM8974PRO-AB",  "801",       SOC_SNAPDRAGON, len, soc, 28)
  CHECK_SOC(tmp, "MSM8974PRO-AC",  "801",       SOC_SNAPDRAGON, len, soc, 28)
  CHECK_SOC(tmp, "APQ8084",        "805",       SOC_SNAPDRAGON, len, soc, 28)
  CHECK_SOC(tmp, "MSM8992",        "808",       SOC_SNAPDRAGON, len, soc, 20)
  CHECK_SOC(tmp, "MSM8994",        "810",       SOC_SNAPDRAGON, len, soc, 20)
  CHECK_SOC(tmp, "MSM8996",        "820",       SOC_SNAPDRAGON, len, soc, 14)
  CHECK_SOC(tmp, "MSM8996 Pro A",  "821",       SOC_SNAPDRAGON, len, soc, 14)
  CHECK_SOC(tmp, "MSM8996 Pro AB", "821 AB",    SOC_SNAPDRAGON, len, soc, 14)
  CHECK_SOC(tmp, "MSM8995",        "835",       SOC_SNAPDRAGON, len, soc, 10)
  //CHECK_SOC(tmp, "?",            "845",       SOC_SNAPDRAGON, len, soc, 10)
  //CHECK_SOC(tmp, "?",            "850",       SOC_SNAPDRAGON, len, soc, 10)
  CHECK_SOC(tmp, "SM8150",         "855",       SOC_SNAPDRAGON, len, soc,  7)
  //CHECK_SOC(tmp, "?",            "855+",      SOC_SNAPDRAGON, len, soc,  7)
  CHECK_SOC(tmp, "SM8250",         "865",       SOC_SNAPDRAGON, len, soc,  7)
  CHECK_SOC(tmp, "SM8250-AB",      "865+",      SOC_SNAPDRAGON, len, soc,  7)
  SOC_END
}

struct system_on_chip* parse_soc_from_string(struct system_on_chip* soc) {
  char* raw_name = soc->raw_name;
  
  if (match_qualcomm(raw_name, soc))
    return soc;
  
  return soc;
}

#ifdef __ANDROID__
#include <sys/system_properties.h>

static inline int android_property_get(const char* key, char* value) {
  return __system_property_get(key, value);
}

struct system_on_chip* guess_soc_from_android() {
  struct system_on_chip* soc = NULL;
  char tmp[100];
  int property_len = 0;
  
  property_len = android_property_get("ro.mediatek.platform", (char *) &tmp);
  if(property_len > 0) {
    soc = malloc(sizeof(struct system_on_chip));
    soc->raw_name = malloc(sizeof(char) * (property_len + 1));
    strncpy(soc->raw_name, tmp, property_len + 1);
    soc->raw_name[property_len] = '\0';
    soc->soc_vendor = SOC_UNKNOWN;
    return parse_soc_from_string(soc);
  }
  
  property_len = android_property_get("ro.product.board", (char *) &tmp);
  if(property_len > 0) {    
    soc = malloc(sizeof(struct system_on_chip));
    soc->raw_name = malloc(sizeof(char) * (property_len + 1));
    strncpy(soc->raw_name, tmp, property_len + 1);
    soc->raw_name[property_len] = '\0';
    soc->soc_vendor = SOC_UNKNOWN;
    return parse_soc_from_string(soc);
  }    
  
  return soc;
}
#endif

struct system_on_chip* guess_soc_from_cpuinfo() {
  struct system_on_chip* soc = NULL;
  
  char* tmp = get_hardware_from_cpuinfo(&strlen);
  if(tmp != NULL) {
    soc = malloc(sizeof(struct system_on_chip));  
    soc->raw_name = tmp;
    soc->soc_vendor = SOC_UNKNOWN;
    return parse_soc_from_string(soc);
  }
  
  return NULL;
}

struct system_on_chip* get_soc() {
  struct system_on_chip* soc = NULL;  
  
  soc = guess_soc_from_cpuinfo();
  if(soc == NULL) {
    printWarn("SoC detection failed using /proc/cpuinfo");
#ifdef __ANDROID__
    soc = guess_soc_from_android();
    if(soc == NULL) {
      printWarn("SoC detection failed using Android");
    }
#endif
  }

  if(soc == NULL) {
    soc = malloc(sizeof(struct system_on_chip));
    soc->raw_name = malloc(sizeof(char) * (strlen(STRING_UNKNOWN)+1));
    snprintf(soc->raw_name, strlen(STRING_UNKNOWN)+1, STRING_UNKNOWN);
  }
  
  return soc;    
}

char* get_soc_name(struct system_on_chip* soc) {
  if(soc->soc_vendor == SOC_UNKNOWN)
    return soc->raw_name;
  return soc->soc_name;
}

