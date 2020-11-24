#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "soc.h"
#include "../common/udev.h"
#include "../common/global.h"

#define STRING_UNKNOWN    "Unknown"

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
    return soc;
  }
  
  property_len = android_property_get("ro.product.board", (char *) &tmp);
  if(property_len > 0) {    
    soc = malloc(sizeof(struct system_on_chip));
    soc->raw_name = malloc(sizeof(char) * (property_len + 1));
    strncpy(soc->raw_name, tmp, property_len + 1);
    soc->raw_name[property_len] = '\0';
    return soc;
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
  }
  
  return soc;
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
