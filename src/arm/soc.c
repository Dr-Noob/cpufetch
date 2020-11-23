#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "soc.h"

#define STRING_UNKNOWN    "Unknown"

#ifdef __ANDROID__
#include <sys/system_properties.h>

static inline int android_property_get(const char* key, char* value) {
  return __system_property_get(key, value);
}

char* android_guess_soc() {
  char* raw_name = NULL;
  char tmp[100];
  int property_len = 0;
  
  property_len = android_property_get("ro.mediatek.platform", (char *) &tmp);
  if(property_len > 0) {
    raw_name = malloc(sizeof(char) * (property_len + 1));
    strcpy(raw_name, tmp);
    raw_name[property_len] = '\0';
    return raw_name;
  }
  
  property_len = android_property_get("ro.product.board", (char *) &tmp);
  if(property_len > 0) {    
    raw_name = malloc(sizeof(char) * (property_len + 1));
    strcpy(raw_name, tmp);
    raw_name[property_len] = '\0';
    return raw_name;
  }    
  
  return NULL;
}
#endif

struct system_on_chip* get_soc() {
  struct system_on_chip* soc = malloc(sizeof(struct system_on_chip));
  soc->raw_name = NULL;
  
#ifdef __ANDROID__
  soc->raw_name = android_guess_soc();
#endif

  if(soc->raw_name == NULL) {
    soc->raw_name = malloc(sizeof(char) * (strlen(STRING_UNKNOWN)+1));
    snprintf(soc->raw_name, strlen(STRING_UNKNOWN)+1, STRING_UNKNOWN);
  }
  
  return soc;    
}
