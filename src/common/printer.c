#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "printer.h"
#include "ascii.h"
#include "../common/global.h"
#include "../common/cpu.h"

#ifdef ARCH_X86
  #include "../x86/uarch.h"
  #include "../x86/cpuid.h"
#else
  #include "../arm/uarch.h"
  #include "../arm/midr.h"
  #include "../arm/soc.h"
#endif

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

#define MAX_ATTRIBUTES      100

#define COL_NONE            ""
#define COL_INTEL_FANCY_1   "\x1b[46;1m"
#define COL_INTEL_FANCY_2   "\x1b[47;1m"
#define COL_INTEL_FANCY_3   "\x1b[36;1m"
#define COL_INTEL_FANCY_4   "\x1b[37;1m"
#define COL_INTEL_RETRO_1   "\x1b[36;1m"
#define COL_INTEL_RETRO_2   "\x1b[37;1m"

#define COL_AMD_FANCY_1     "\x1b[47;1m"
#define COL_AMD_FANCY_2     "\x1b[42;1m"
#define COL_AMD_FANCY_3     "\x1b[37;1m"
#define COL_AMD_FANCY_4     "\x1b[32;1m"
#define COL_AMD_RETRO_1     "\x1b[37;1m"
#define COL_AMD_RETRO_2     "\x1b[32;1m"

#define COL_ARM_FANCY_1     "\x1b[46;1m"
#define COL_ARM_FANCY_2     "\x1b[46;1m"
#define COL_ARM_FANCY_3     "\x1b[37;1m"
#define COL_ARM_FANCY_4     "\x1b[36;1m"
#define COL_ARM_RETRO_1     "\x1b[36;1m"
#define COL_ARM_RETRO_2     "\x1b[37;1m"

#define COL_UNKNOWN_FANCY_1 "\x1b[47;1m"
#define COL_UNKNOWN_FANCY_2 "\x1b[47;1m"
#define COL_UNKNOWN_FANCY_3 "\x1b[37;1m"
#define COL_UNKNOWN_FANCY_4 "\x1b[31;1m"
#define COL_UNKNOWN_RETRO   "\x1b[32;0m"
#define RESET               "\x1b[m"

enum {
#ifdef ARCH_X86    
  ATTRIBUTE_NAME,
#elif ARCH_ARM
  ATTRIBUTE_SOC,
  ATTRIBUTE_CPU_NUM,
#endif
  ATTRIBUTE_HYPERVISOR,
  ATTRIBUTE_UARCH,
  ATTRIBUTE_TECHNOLOGY,
  ATTRIBUTE_FREQUENCY,
  ATTRIBUTE_SOCKETS,
  ATTRIBUTE_NCORES,
  ATTRIBUTE_NCORES_DUAL,
#ifdef ARCH_X86
  ATTRIBUTE_AVX,
  ATTRIBUTE_FMA,
#endif
  ATTRIBUTE_L1i,
  ATTRIBUTE_L1d,
  ATTRIBUTE_L2,
  ATTRIBUTE_L3,
  ATTRIBUTE_PEAK
};

static const char* ATTRIBUTE_FIELDS [] = {
#ifdef ARCH_X86    
  "Name:",
 #elif ARCH_ARM 
  "SoC:",
  "",
#endif
  "Hypervisor:",
  "Microarchitecture:",
  "Technology:",
  "Max Frequency:",
  "Sockets:",
  "Cores:",
  "Cores (Total):",
#ifdef ARCH_X86
  "AVX:",
  "FMA:",
#endif
  "L1i Size:",
  "L1d Size:",
  "L2 Size:",
  "L3 Size:",
  "Peak Performance:",
};

struct attribute {
  int type;
  char* value;    
};

struct ascii {
  char art[NUMBER_OF_LINES][LINE_SIZE];
  char color1_ascii[100];
  char color2_ascii[100];
  char color1_text[100];
  char color2_text[100];
  char ascii_chars[2];
  char reset[100];
  struct attribute** attributes;
  uint32_t n_attributes_set;
  VENDOR vendor;
  STYLE style;
};

void setAttribute(struct ascii* art, int type, char* value) {
  art->attributes[art->n_attributes_set]->value = value;
  art->attributes[art->n_attributes_set]->type = type;
  art->n_attributes_set++;
  
  if(art->n_attributes_set > MAX_ATTRIBUTES) {
    printBug("Set %d attributes, while max value is %d!", art->n_attributes_set, MAX_ATTRIBUTES);
  }
}

char* rgb_to_ansi(struct color* c, bool background, bool bold) {
  char* str = malloc(sizeof(char) * 100);
  if(background) {
    snprintf(str, 44, "\x1b[48;2;%.3d;%.3d;%.3dm", c->R, c->G, c->B);
  }
  else {
    if(bold)
      snprintf(str, 48, "\x1b[1m\x1b[38;2;%.3d;%.3d;%.3dm", c->R, c->G, c->B);
    else
      snprintf(str, 44, "\x1b[38;2;%.3d;%.3d;%.3dm", c->R, c->G, c->B);
  }

  return str;
}

struct ascii* set_ascii(VENDOR cpuVendor, STYLE style, struct colors* cs) {
  char *COL_FANCY_1, *COL_FANCY_2, *COL_FANCY_3, *COL_FANCY_4, *COL_RETRO_1, *COL_RETRO_2, *COL_RETRO_3, *COL_RETRO_4;
  struct ascii* art = malloc(sizeof(struct ascii));
  art->n_attributes_set = 0;
  art->vendor = cpuVendor;
  art->attributes = malloc(sizeof(struct attribute *) * MAX_ATTRIBUTES);
  for(uint32_t i=0; i < MAX_ATTRIBUTES; i++) {
    art->attributes[i] = malloc(sizeof(struct attribute));
    art->attributes[i]->type = 0;
    art->attributes[i]->value = NULL;
  }
  strcpy(art->reset,RESET);

  if(art->vendor == CPU_VENDOR_INTEL) {
    COL_FANCY_1 = COL_INTEL_FANCY_1;
    COL_FANCY_2 = COL_INTEL_FANCY_2;
    COL_FANCY_3 = COL_INTEL_FANCY_3;
    COL_FANCY_4 = COL_INTEL_FANCY_4;
    COL_RETRO_1 = COL_INTEL_RETRO_1;
    COL_RETRO_2 = COL_INTEL_RETRO_2;
    COL_RETRO_3 = COL_INTEL_RETRO_1;
    COL_RETRO_4 = COL_INTEL_RETRO_2;
    art->ascii_chars[0] = '#';
  }
  else if(art->vendor == CPU_VENDOR_AMD) {
    COL_FANCY_1 = COL_AMD_FANCY_1;
    COL_FANCY_2 = COL_AMD_FANCY_2;
    COL_FANCY_3 = COL_AMD_FANCY_3;
    COL_FANCY_4 = COL_AMD_FANCY_4;
    COL_RETRO_1 = COL_AMD_RETRO_1;
    COL_RETRO_2 = COL_AMD_RETRO_2;
    COL_RETRO_3 = COL_AMD_RETRO_1;
    COL_RETRO_4 = COL_AMD_RETRO_2;
    art->ascii_chars[0] = '@';
  }
  else if(art->vendor == CPU_VENDOR_ARM) {
    COL_FANCY_1 = COL_ARM_FANCY_1;
    COL_FANCY_2 = COL_ARM_FANCY_2;
    COL_FANCY_3 = COL_ARM_FANCY_3;
    COL_FANCY_4 = COL_ARM_FANCY_4;
    COL_RETRO_1 = COL_ARM_RETRO_1;
    COL_RETRO_2 = COL_ARM_RETRO_2;
    COL_RETRO_3 = COL_ARM_RETRO_1;
    COL_RETRO_4 = COL_ARM_RETRO_2;
    art->ascii_chars[0] = '#';
  }
  else {
    COL_FANCY_1 = COL_UNKNOWN_FANCY_1;
    COL_FANCY_2 = COL_UNKNOWN_FANCY_2;
    COL_FANCY_3 = COL_UNKNOWN_FANCY_3;
    COL_FANCY_4 = COL_UNKNOWN_FANCY_4;
    COL_RETRO_1 = COL_UNKNOWN_RETRO;
    COL_RETRO_2 = COL_UNKNOWN_RETRO;
    COL_RETRO_3 = COL_UNKNOWN_RETRO;
    COL_RETRO_4 = COL_UNKNOWN_RETRO;
    art->ascii_chars[0] = '#';
  }
  art->ascii_chars[1] = '#';

  #ifdef _WIN32
    HANDLE std_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD console_mode;

    // Attempt to enable the VT100-processing flag
    GetConsoleMode(std_handle, &console_mode);
    SetConsoleMode(std_handle, console_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    // Get the console mode flag again, to see if it successfully enabled it
    GetConsoleMode(std_handle, &console_mode);
  #endif

  if(style == STYLE_EMPTY) {
    #ifdef _WIN32
      // Use fancy style if VT100-processing is enabled,
      // or legacy style in other case
      art->style = (console_mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) ? STYLE_FANCY : STYLE_LEGACY;
    #else
      art->style = STYLE_FANCY;
    #endif
  }
  else {
    art->style = style;
  }

  switch(art->style) {
    case STYLE_LEGACY:
      strcpy(art->color1_ascii,COL_NONE);
      strcpy(art->color2_ascii,COL_NONE);
      strcpy(art->color1_text,COL_NONE);
      strcpy(art->color2_text,COL_NONE);
      art->reset[0] = '\0';
      break;
    case STYLE_FANCY:
      if(cs != NULL) {
        COL_FANCY_1 = rgb_to_ansi(cs->c1, true, true);
        COL_FANCY_2 = rgb_to_ansi(cs->c2, true, true);
        COL_FANCY_3 = rgb_to_ansi(cs->c3, false, true);
        COL_FANCY_4 = rgb_to_ansi(cs->c4, false, true);
      }
      art->ascii_chars[0] = ' ';
      art->ascii_chars[1] = ' ';
      strcpy(art->color1_ascii,COL_FANCY_1);
      strcpy(art->color2_ascii,COL_FANCY_2);
      strcpy(art->color1_text,COL_FANCY_3);
      strcpy(art->color2_text,COL_FANCY_4);
      if(cs != NULL) {
        free(COL_FANCY_1);
        free(COL_FANCY_2);
        free(COL_FANCY_3);
        free(COL_FANCY_4);
      }
      break;
    case STYLE_RETRO:
      if(cs != NULL) {
        COL_RETRO_1 = rgb_to_ansi(cs->c1, false, true);
        COL_RETRO_2 = rgb_to_ansi(cs->c2, false, true);
        COL_RETRO_3 = rgb_to_ansi(cs->c3, false, true);
        COL_RETRO_4 = rgb_to_ansi(cs->c4, false, true);
      }
      strcpy(art->color1_ascii,COL_RETRO_1);
      strcpy(art->color2_ascii,COL_RETRO_2);
      strcpy(art->color1_text,COL_RETRO_3);
      strcpy(art->color2_text,COL_RETRO_4);
      if(cs != NULL) {
        free(COL_RETRO_1);
        free(COL_RETRO_2);
        free(COL_RETRO_3);
        free(COL_RETRO_4);
      }
      break;
    case STYLE_INVALID:
    default:
      printBug("Found invalid style (%d)", art->style);
      return NULL;
  }

  char tmp[NUMBER_OF_LINES*LINE_SIZE];
  if(art->vendor == CPU_VENDOR_INTEL)
    strcpy(tmp, INTEL_ASCII);
  else if(art->vendor == CPU_VENDOR_AMD)
    strcpy(tmp, AMD_ASCII);
  else if(art->vendor == CPU_VENDOR_ARM)
    strcpy(tmp, ARM_ASCII);
  else
    strcpy(tmp, UNKNOWN_ASCII);

  for(int i=0; i < NUMBER_OF_LINES; i++)
    strncpy(art->art[i], tmp + i*LINE_SIZE, LINE_SIZE);

  return art;
}

uint32_t longest_attribute_length(struct ascii* art) {
  uint32_t max = 0;
  uint64_t len = 0;

  for(uint32_t i=0; i < art->n_attributes_set; i++) {
    if(art->attributes[i]->value != NULL) {
      len = strlen(ATTRIBUTE_FIELDS[art->attributes[i]->type]);
      if(len > max) max = len;
    }
  }

  return max;
}

#ifdef ARCH_X86
void print_ascii_intel(struct ascii* art, uint32_t la) {  
  int attr_to_print = 0;
  int attr_type;
  char* attr_value;
  uint32_t space_right;
  uint32_t space_up = (NUMBER_OF_LINES - art->n_attributes_set)/2;
  uint32_t space_down = NUMBER_OF_LINES - art->n_attributes_set - space_up;
  bool flag = false;

  printf("\n");
  for(uint32_t n=0;n<NUMBER_OF_LINES;n++) {

    for(int i=0;i<LINE_SIZE;i++) {
      if(flag) {
        if(art->art[n][i] == ' ') {
          flag = false;
          printf("%s%c%s", art->color2_ascii, art->ascii_chars[1], art->reset);
        }
        else {
          printf("%s%c%s", art->color1_ascii, art->ascii_chars[0], art->reset);
        }
      }
      else {
        if(art->art[n][i] != ' ' && art->art[n][i] != '\0') {
          flag = true;
          printf("%c",' ');
        }
        else
          printf("%c",' ');
      }
    }

    if(n > space_up-1 && n < NUMBER_OF_LINES-space_down) {
      attr_type = art->attributes[attr_to_print]->type;
      attr_value = art->attributes[attr_to_print]->value;
      attr_to_print++;
      
      space_right = 1 + (la - strlen(ATTRIBUTE_FIELDS[attr_type]));
      printf("%s%s%s%*s%s%s%s\n", art->color1_text, ATTRIBUTE_FIELDS[attr_type], art->reset, space_right, "", art->color2_text, attr_value, art->reset);
    }
    else printf("\n");
  }
  printf("\n");
}

void print_ascii_amd(struct ascii* art, uint32_t la) {
  int attr_to_print = 0;
  int attr_type;
  char* attr_value;
  uint32_t space_right;
  uint32_t space_up = (NUMBER_OF_LINES - art->n_attributes_set)/2;
  uint32_t space_down = NUMBER_OF_LINES - art->n_attributes_set - space_up;

  printf("\n");
  for(uint32_t n=0;n<NUMBER_OF_LINES;n++) {
    for(int i=0;i<LINE_SIZE;i++) {
      if(art->art[n][i] == '@')
        printf("%s%c%s", art->color1_ascii, art->ascii_chars[0], art->reset);
      else if(art->art[n][i] == '#')
        printf("%s%c%s", art->color2_ascii, art->ascii_chars[1], art->reset);
      else
        printf("%c",art->art[n][i]);
    }

    if(n > space_up-1 && n < NUMBER_OF_LINES-space_down) {
      attr_type = art->attributes[attr_to_print]->type;
      attr_value = art->attributes[attr_to_print]->value;
      attr_to_print++;
      
      space_right = 1 + (la - strlen(ATTRIBUTE_FIELDS[attr_type]));
      printf("%s%s%s%*s%s%s%s\n", art->color1_text, ATTRIBUTE_FIELDS[attr_type], art->reset, space_right, "", art->color2_text, attr_value, art->reset);
    }
    else printf("\n");
  }
  printf("\n");

}

void print_ascii(struct ascii* art) {
  uint32_t longest_attribute = longest_attribute_length(art);
  
  if(art->vendor == CPU_VENDOR_INTEL)
    print_ascii_intel(art, longest_attribute);
  else if(art->vendor == CPU_VENDOR_AMD)
    print_ascii_amd(art, longest_attribute);
  else {
    printBug("Invalid CPU vendor: %d\n", art->vendor);
  }
  
}

bool print_cpufetch_x86(struct ascii* art, struct cpuInfo* cpu, struct colors* cs) {
  char* uarch = get_str_uarch(cpu);
  char* manufacturing_process = get_str_process(cpu);
  char* sockets = get_str_sockets(cpu->topo);
  char* max_frequency = get_str_freq(cpu->freq);
  char* n_cores = get_str_topology(cpu, cpu->topo, false);
  char* n_cores_dual = get_str_topology(cpu, cpu->topo, true);
  char* cpu_name = get_str_cpu_name(cpu);
  char* avx = get_str_avx(cpu);
  char* fma = get_str_fma(cpu);


  char* l1i = get_str_l1i(cpu->cach);
  char* l1d = get_str_l1d(cpu->cach);
  char* l2 = get_str_l2(cpu->cach);
  char* l3 = get_str_l3(cpu->cach);
  char* pp = get_str_peak_performance(cpu,cpu->topo,get_freq(cpu->freq));

  setAttribute(art,ATTRIBUTE_NAME,cpu_name);
  if(cpu->hv->present) {
    setAttribute(art, ATTRIBUTE_HYPERVISOR, cpu->hv->hv_name);
  }
  setAttribute(art,ATTRIBUTE_UARCH,uarch);
  setAttribute(art,ATTRIBUTE_TECHNOLOGY,manufacturing_process);
  setAttribute(art,ATTRIBUTE_FREQUENCY,max_frequency);
  uint32_t socket_num = get_nsockets(cpu->topo);
  if (socket_num > 1) {
    setAttribute(art, ATTRIBUTE_SOCKETS, sockets);
    setAttribute(art, ATTRIBUTE_NCORES,n_cores);
    setAttribute(art, ATTRIBUTE_NCORES_DUAL, n_cores_dual);
  }
  else {
    setAttribute(art,ATTRIBUTE_NCORES,n_cores);
  }
  setAttribute(art,ATTRIBUTE_AVX,avx);
  setAttribute(art,ATTRIBUTE_FMA,fma);  
  setAttribute(art,ATTRIBUTE_L1i,l1i);
  setAttribute(art,ATTRIBUTE_L1d,l1d);
  setAttribute(art,ATTRIBUTE_L2,l2);
  if(l3 != NULL) {
    setAttribute(art,ATTRIBUTE_L3,l3);
  }
  setAttribute(art,ATTRIBUTE_PEAK,pp);
  
  if(art->n_attributes_set > NUMBER_OF_LINES) {
    printBug("The number of attributes set is bigger than the max that can be displayed");
    return false;
  }

  print_ascii(art);

  free(manufacturing_process);
  free(max_frequency);
  free(sockets);
  free(n_cores);
  free(n_cores_dual);
  free(avx);
  free(fma);
  free(l1i);
  free(l1d);
  free(l2);
  free(l3);
  free(pp);

  free(art->attributes);
  free(art);

  if(cs != NULL) free_colors_struct(cs);
  free_cache_struct(cpu->cach);
  free_topo_struct(cpu->topo);
  free_freq_struct(cpu->freq);
  free_cpuinfo_struct(cpu);

  return true;
}
#endif

#ifdef ARCH_ARM
void print_ascii_arm(struct ascii* art, uint32_t la) {
  int attr_to_print = 0;
  int attr_type;
  char* attr_value;
  uint32_t space_right;
  uint32_t space_up = (NUMBER_OF_LINES - art->n_attributes_set)/2;
  uint32_t space_down = NUMBER_OF_LINES - art->n_attributes_set - space_up;
  bool add_space = false;

  printf("\n");
  for(uint32_t n=0;n<NUMBER_OF_LINES;n++) {
    for(int i=0;i<LINE_SIZE;i++) {
      if(art->art[n][i] == '#')
        printf("%s%c%s", art->color1_ascii, art->ascii_chars[0], art->reset);
      else
        printf("%c",art->art[n][i]);
    }

    if(n > space_up-1 && n < NUMBER_OF_LINES-space_down) {
      attr_type = art->attributes[attr_to_print]->type;
      attr_value = art->attributes[attr_to_print]->value;
      attr_to_print++;
       
      if(attr_type == ATTRIBUTE_PEAK) {
        add_space = false;
      }
      if(attr_type == ATTRIBUTE_CPU_NUM) {
        printf("%s%s%s\n", art->color1_text, attr_value, art->reset);
        add_space = true;
      }      
      else {
        if(add_space) {
          space_right = 1 + (la - strlen(ATTRIBUTE_FIELDS[attr_type]));
          printf("  %s%s%s%*s%s%s%s\n", art->color1_text, ATTRIBUTE_FIELDS[attr_type], art->reset, space_right, "", art->color2_text, attr_value, art->reset);
        }
        else {
          space_right = 2 + 1 + (la - strlen(ATTRIBUTE_FIELDS[attr_type]));
          printf("%s%s%s%*s%s%s%s\n", art->color1_text, ATTRIBUTE_FIELDS[attr_type], art->reset, space_right, "", art->color2_text, attr_value, art->reset);
        }
      }
    }
    else printf("\n");
  }
  printf("\n");

}

void print_ascii(struct ascii* art) {
  uint32_t longest_attribute = longest_attribute_length(art);
  
  if(art->vendor == CPU_VENDOR_ARM)
    print_ascii_arm(art, longest_attribute);
  else {
    printWarn("Invalid CPU vendor: %d\n", art->vendor);
    print_ascii_arm(art, longest_attribute);
  }
  
}

bool print_cpufetch_arm(struct ascii* art, struct cpuInfo* cpu, struct colors* cs) {    
  char* manufacturing_process = get_str_process(cpu);
  char* soc_name = get_soc_name(cpu->soc);
  setAttribute(art,ATTRIBUTE_SOC,soc_name);
  setAttribute(art,ATTRIBUTE_TECHNOLOGY,manufacturing_process);
  
  if(cpu->num_cpus == 1) {
    char* uarch = get_str_uarch(cpu);
    char* max_frequency = get_str_freq(cpu->freq);
    char* n_cores = get_str_topology(cpu, cpu->topo, false);
    char* l1i = get_str_l1i(cpu->cach);
    char* l1d = get_str_l1d(cpu->cach);
    char* l2 = get_str_l2(cpu->cach);
    char* l3 = get_str_l3(cpu->cach);
  
    setAttribute(art,ATTRIBUTE_UARCH,uarch);
    setAttribute(art,ATTRIBUTE_FREQUENCY,max_frequency);
    setAttribute(art,ATTRIBUTE_NCORES,n_cores);
    setAttribute(art,ATTRIBUTE_L1i,l1i);
    setAttribute(art,ATTRIBUTE_L1d,l1d);
    setAttribute(art,ATTRIBUTE_L2,l2);
    if(l3 != NULL) {
      setAttribute(art,ATTRIBUTE_L3,l3);
    }
  }
  else {
    struct cpuInfo* ptr = cpu;
    for(int i = 0; i < cpu->num_cpus; ptr = ptr->next_cpu, i++) {
      char* uarch = get_str_uarch(ptr);
      char* max_frequency = get_str_freq(ptr->freq);
      char* n_cores = get_str_topology(ptr, ptr->topo, false);
      char* l1i = get_str_l1i(ptr->cach);
      char* l1d = get_str_l1d(ptr->cach);
      char* l2 = get_str_l2(ptr->cach);
      char* l3 = get_str_l3(ptr->cach);
      
      char* cpu_num = malloc(sizeof(char) * 9);
      sprintf(cpu_num, "CPU %d:", i+1);
      setAttribute(art, ATTRIBUTE_CPU_NUM, cpu_num);
      setAttribute(art, ATTRIBUTE_UARCH, uarch);
      setAttribute(art, ATTRIBUTE_FREQUENCY, max_frequency);
      setAttribute(art, ATTRIBUTE_NCORES, n_cores);
      setAttribute(art, ATTRIBUTE_L1i, l1i);
      setAttribute(art, ATTRIBUTE_L1d, l1d);
      setAttribute(art, ATTRIBUTE_L2, l2); 
      if(l3 != NULL) {
        setAttribute(art,ATTRIBUTE_L3,l3);
      }
    }
  }
  char* pp = get_str_peak_performance(cpu);
  setAttribute(art,ATTRIBUTE_PEAK,pp);
  
  if(art->n_attributes_set > NUMBER_OF_LINES) {
    printBug("The number of attributes set is bigger than the max that can be displayed");
    return false;
  }
  if(cpu->hv->present)
    setAttribute(art, ATTRIBUTE_HYPERVISOR, cpu->hv->hv_name);

  print_ascii(art);

  free(manufacturing_process);  
  free(pp);

  free(art->attributes);
  free(art);

  if(cs != NULL) free_colors_struct(cs);
  free_cache_struct(cpu->cach);
  free_topo_struct(cpu->topo);
  free_cpuinfo_struct(cpu);

  return true;
}
#endif

bool print_cpufetch(struct cpuInfo* cpu, STYLE s, struct colors* cs) {
  // Sanity check of ASCII arts
  for(int i=0; i < 4; i++) {
    const char* ascii = ASCII_ARRAY[i];
    if(strlen(ascii) != (NUMBER_OF_LINES * LINE_SIZE)-1) {
      printBug("ASCII art %d is wrong! ASCII length: %d, expected length: %d", i, strlen(ascii), (NUMBER_OF_LINES * LINE_SIZE)-1);
      return false;
    }
  }
    
  struct ascii* art = set_ascii(get_cpu_vendor(cpu), s, cs);
  if(art == NULL)
    return false;
  
#ifdef ARCH_X86
  return print_cpufetch_x86(art, cpu, cs);
#elif ARCH_ARM
  return print_cpufetch_arm(art, cpu, cs);  
#endif
}
