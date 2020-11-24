#ifndef __SOC__
#define __SOC__

typedef int SOC;

struct system_on_chip {
  SOC soc;
  char* raw_name;    
};

struct system_on_chip* get_soc();
char* get_soc_name(struct system_on_chip* soc);

#endif
