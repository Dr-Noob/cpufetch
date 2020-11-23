#ifndef __SOC__
#define __SOC__

struct system_on_chip {
  char* raw_name;    
};

struct system_on_chip* get_soc();

#endif
