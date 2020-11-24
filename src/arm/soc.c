#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "soc.h"
#include "udev.h"
#include "../common/global.h"

#define STRING_UNKNOWN    "Unknown"

enum {
  SOC_UNKNOWN,
  SOC_MSM8953,
  SOC_MSM8974,
};

static int socs_array[] = {
  SOC_UNKNOWN,
  SOC_MSM8953,
  SOC_MSM8974,
};

static char* socs_string[] = {
  "Unknown",
  "Snapdragon 625",
  "Snapdragon 801",
};

static char* socs_raw_string[] = {
  "",
  "MSM8953",
  "MSM8974",
};

bool match_msm(char* soc_name, struct system_on_chip* soc) {
  char* tmp = strstr(soc_name, "MSM");
  if(tmp == NULL) return false;
  
  char soc_raw_string[8];
  strncpy(soc_raw_string, tmp, 7);
  int len = sizeof(socs_raw_string) / sizeof(socs_raw_string[0]);
  int i=1;
  
  while(i < len && strcmp(soc_raw_string, socs_raw_string[i]) != 0) i++;
  
  if(i != len) {
    soc->soc = socs_array[i];
    return true;
  }
  
  return false;
}

struct system_on_chip* parse_soc_from_string(struct system_on_chip* soc) {
  char* raw_name = soc->raw_name;
  
  if (match_msm(raw_name, soc))
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
    soc->soc = SOC_UNKNOWN;
    return parse_soc_from_string(soc);
  }
  
  property_len = android_property_get("ro.product.board", (char *) &tmp);
  if(property_len > 0) {    
    soc = malloc(sizeof(struct system_on_chip));
    soc->raw_name = malloc(sizeof(char) * (property_len + 1));
    strncpy(soc->raw_name, tmp, property_len + 1);
    soc->raw_name[property_len] = '\0';
    soc->soc = SOC_UNKNOWN;
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
    soc->soc = SOC_UNKNOWN;
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
  if(soc->soc == SOC_UNKNOWN)
    return soc->raw_name;
  return socs_string[soc->soc];
}

