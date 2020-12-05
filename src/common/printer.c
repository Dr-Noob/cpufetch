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

#define max(a,b) (((a)>(b))?(a):(b))
#define MAX_ATTRIBUTES      100

#define COLOR_NONE       ""
#define COLOR_FG_BLACK   "\x1b[30;1m"
#define COLOR_FG_RED     "\x1b[31;1m"
#define COLOR_FG_GREEN   "\x1b[32;1m"
#define COLOR_FG_YELLOW  "\x1b[33;1m"
#define COLOR_FG_BLUE    "\x1b[34;1m"
#define COLOR_FG_MAGENTA "\x1b[35;1m"
#define COLOR_FG_CYAN    "\x1b[36;1m"
#define COLOR_FG_WHITE   "\x1b[37;1m"
#define COLOR_BG_BLACK   "\x1b[40;1m"
#define COLOR_BG_RED     "\x1b[41;1m"
#define COLOR_BG_GREEN   "\x1b[42;1m"
#define COLOR_BG_YELLOW  "\x1b[43;1m"
#define COLOR_BG_BLUE    "\x1b[44;1m"
#define COLOR_BG_MAGENTA "\x1b[45;1m"
#define COLOR_BG_CYAN    "\x1b[46;1m"
#define COLOR_BG_WHITE   "\x1b[47;1m"
#define COLOR_RESET      "\x1b[m"

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
#elif ARCH_ARM
  ATTRIBUTE_FEATURES,
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
#elif ARCH_ARM
  "Features: ",  
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
  char art[NUMBER_OF_LINES][LINE_SIZE+1];
  char color1_ascii[100];
  char color2_ascii[100];
  char color1_text[100];
  char color2_text[100];
  char ascii_chars[2];
  char reset[100];
  struct attribute** attributes;
  uint32_t n_attributes_set;
  uint32_t additional_spaces;
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

struct ascii* set_ascii(VENDOR vendor, STYLE style, struct colors* cs) {
  char *COL_FANCY_1, *COL_FANCY_2, *COL_FANCY_3, *COL_FANCY_4, *COL_RETRO_1, *COL_RETRO_2, *COL_RETRO_3, *COL_RETRO_4;
  struct ascii* art = malloc(sizeof(struct ascii));
  art->n_attributes_set = 0;
  art->additional_spaces = 0;
  art->vendor = vendor;
  art->attributes = malloc(sizeof(struct attribute *) * MAX_ATTRIBUTES);
  for(uint32_t i=0; i < MAX_ATTRIBUTES; i++) {
    art->attributes[i] = malloc(sizeof(struct attribute));
    art->attributes[i]->type = 0;
    art->attributes[i]->value = NULL;
  }
  strcpy(art->reset, COLOR_RESET);

#ifdef ARCH_X86  
  if(art->vendor == CPU_VENDOR_INTEL) {
    COL_FANCY_1 = COLOR_BG_CYAN;
    COL_FANCY_2 = COLOR_BG_WHITE;
    COL_FANCY_3 = COLOR_FG_CYAN;
    COL_FANCY_4 = COLOR_FG_WHITE;
    art->ascii_chars[0] = '#';
  }
  else if(art->vendor == CPU_VENDOR_AMD) {
    COL_FANCY_1 = COLOR_BG_WHITE;
    COL_FANCY_2 = COLOR_BG_GREEN;
    COL_FANCY_3 = COLOR_FG_WHITE;
    COL_FANCY_4 = COLOR_FG_GREEN;    
    art->ascii_chars[0] = '@';
  }
  else {
    printBug("Invalid CPU vendor in set_ascii (%d)", art->vendor);    
    return NULL;
  }
#elif ARCH_ARM
  if(art->vendor == SOC_VENDOR_SNAPDRAGON) {
    COL_FANCY_1 = COLOR_BG_RED;
    COL_FANCY_2 = COLOR_BG_WHITE;
    COL_FANCY_3 = COLOR_FG_RED;
    COL_FANCY_4 = COLOR_FG_WHITE;
    art->ascii_chars[0] = '@';
  }
  else if(art->vendor == SOC_VENDOR_MEDIATEK) {
    COL_FANCY_1 = COLOR_BG_BLUE;
    COL_FANCY_2 = COLOR_BG_YELLOW;
    COL_FANCY_3 = COLOR_FG_WHITE;
    COL_FANCY_4 = COLOR_FG_BLUE;
    art->ascii_chars[0] = '@';
  }
  else if(art->vendor == SOC_VENDOR_EXYNOS) {
    COL_FANCY_1 = COLOR_BG_BLUE;
    COL_FANCY_2 = COLOR_BG_WHITE;
    COL_FANCY_3 = COLOR_FG_BLUE;
    COL_FANCY_4 = COLOR_FG_WHITE;
    art->ascii_chars[0] = '@';
  }
  else if(art->vendor == SOC_VENDOR_KIRIN) {
    COL_FANCY_1 = COLOR_BG_WHITE;
    COL_FANCY_2 = COLOR_BG_RED;
    COL_FANCY_3 = COLOR_FG_WHITE;
    COL_FANCY_4 = COLOR_FG_RED;
    art->ascii_chars[0] = '@';
  }
  else if(art->vendor == SOC_VENDOR_BROADCOM) {
    COL_FANCY_1 = COLOR_BG_WHITE;
    COL_FANCY_2 = COLOR_BG_RED;
    COL_FANCY_3 = COLOR_FG_WHITE;
    COL_FANCY_4 = COLOR_FG_RED;
    art->ascii_chars[0] = '@';
  }
  else {
    COL_FANCY_1 = COLOR_BG_CYAN;
    COL_FANCY_2 = COLOR_BG_CYAN;
    COL_FANCY_3 = COLOR_FG_WHITE;
    COL_FANCY_4 = COLOR_FG_CYAN;
    art->ascii_chars[0] = '#';
  }
#endif
  COL_RETRO_1 = COL_FANCY_3;
  COL_RETRO_2 = COL_FANCY_4;
  COL_RETRO_3 = COL_RETRO_1;
  COL_RETRO_4 = COL_RETRO_2;
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
      strcpy(art->color1_ascii, COLOR_NONE);
      strcpy(art->color2_ascii, COLOR_NONE);
      strcpy(art->color1_text, COLOR_NONE);
      strcpy(art->color2_text, COLOR_NONE);
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

  char tmp[NUMBER_OF_LINES * LINE_SIZE + 1];
#ifdef ARCH_X86  
  if(art->vendor == CPU_VENDOR_INTEL)
    strcpy(tmp, INTEL_ASCII);
  else if(art->vendor == CPU_VENDOR_AMD)
    strcpy(tmp, AMD_ASCII);
  else
    strcpy(tmp, UNKNOWN_ASCII);  
#elif ARCH_ARM  
  if(art->vendor == SOC_VENDOR_SNAPDRAGON)
    strcpy(tmp, SNAPDRAGON_ASCII);
  else if(art->vendor == SOC_VENDOR_MEDIATEK)
    strcpy(tmp, MEDIATEK_ASCII);
  else if(art->vendor == SOC_VENDOR_EXYNOS)
    strcpy(tmp, EXYNOS_ASCII);
  else if(art->vendor == SOC_VENDOR_KIRIN)
    strcpy(tmp, KIRIN_ASCII);
  else if(art->vendor == SOC_VENDOR_BROADCOM)
    strcpy(tmp, BROADCOM_ASCII);
  else
    strcpy(tmp, ARM_ASCII);
#endif

  for(int i=0; i < NUMBER_OF_LINES; i++)
    memcpy(art->art[i], tmp + i*LINE_SIZE, LINE_SIZE);

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
void print_algorithm_intel(struct ascii* art, int n, bool* flag) {
  for(int i=0; i < LINE_SIZE; i++) {
    if(*flag) {
      if(art->art[n][i] == ' ') {
        *flag = false;
        printf("%s%c%s", art->color2_ascii, art->ascii_chars[1], art->reset);
      }
      else {
        printf("%s%c%s", art->color1_ascii, art->ascii_chars[0], art->reset);
      }
    }
    else {
      if(art->art[n][i] != ' ' && art->art[n][i] != '\0') {
        *flag = true;
        printf("%c",' ');
      }
      else {
        printf("%c",' ');
      }
    }
  }
}

void print_algorithm_amd(struct ascii* art, int n, bool* flag) {
  *flag = false; // dummy, just silence compiler error
  
  for(int i=0; i < LINE_SIZE; i++) {
    if(art->art[n][i] == '@')
      printf("%s%c%s", art->color1_ascii, art->ascii_chars[0], art->reset);
    else if(art->art[n][i] == '#')
      printf("%s%c%s", art->color2_ascii, art->ascii_chars[1], art->reset);
    else
      printf("%c",art->art[n][i]);
  }
}

void print_ascii_x86(struct ascii* art, uint32_t la, void (*callback_print_algorithm)(struct ascii* art, int i, bool* flag)) {  
  int attr_to_print = 0;
  int attr_type;
  char* attr_value;
  uint32_t space_right;
  uint32_t space_up = (NUMBER_OF_LINES - art->n_attributes_set)/2;
  uint32_t space_down = NUMBER_OF_LINES - art->n_attributes_set - space_up;
  bool flag = false;

  printf("\n");
  for(uint32_t n=0;n<NUMBER_OF_LINES;n++) {
    callback_print_algorithm(art, n, &flag);

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
    print_ascii_x86(art, longest_attribute, &print_algorithm_intel);
  else if(art->vendor == CPU_VENDOR_AMD)
    print_ascii_x86(art, longest_attribute, &print_algorithm_amd);
  else {
    printBug("Invalid CPU vendor: %d\n", art->vendor);
  }
  
}

bool print_cpufetch_x86(struct cpuInfo* cpu, STYLE s, struct colors* cs) {
  struct ascii* art = set_ascii(get_cpu_vendor(cpu), s, cs);
  if(art == NULL)
    return false;  
  
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
void print_algorithm_snapd_mtk(struct ascii* art, int n) {
  for(int i=0; i < LINE_SIZE; i++) {
    if(art->art[n][i] == '@')
      printf("%s%c%s", art->color1_ascii, art->ascii_chars[0], art->reset);
    else if(art->art[n][i] == '#')
      printf("%s%c%s", art->color2_ascii, art->ascii_chars[1], art->reset);  
    else
      printf("%c",art->art[n][i]);    
  }
}

void print_algorithm_samsung(struct ascii* art, int n) {
  int y_margin = 2;
  int x_margin = 2 * y_margin;
  
  for(int i=0; i < LINE_SIZE; i++) {
    if(art->art[n][i] == '#') {
      printf("%s%c%s", art->color1_ascii, art->ascii_chars[0], art->reset);
    }
    else if((n >= y_margin && n < NUMBER_OF_LINES-y_margin) && (i >= x_margin && i < LINE_SIZE-x_margin)) {
      if(art->art[n][i] == '#')
        printf("%s%c%s", art->color1_ascii, art->ascii_chars[0], art->reset);  
      else
        printf("%s%c%s","\x1b[48;2;10;10;10m" COLOR_FG_WHITE, art->art[n][i], art->reset);    
    }
    else
      printf("%c", art->art[n][i]);
  }
}

void print_algorithm_arm(struct ascii* art, int n) {
  for(int i=0; i < LINE_SIZE; i++) {  
    if(art->art[n][i] == '#')
      printf("%s%c%s", art->color1_ascii, art->ascii_chars[0], art->reset);  
    else
      printf("%c",art->art[n][i]);    
  }
}

void print_ascii_arm(struct ascii* art, uint32_t la, void (*callback_print_algorithm)(struct ascii* art, int n)) {  
  int attr_to_print = 0;
  int attr_type;
  char* attr_value;
  uint32_t limit_up;
  uint32_t limit_down;
  
  uint32_t space_right;
  uint32_t space_up = (NUMBER_OF_LINES - art->n_attributes_set)/2;
  uint32_t space_down = NUMBER_OF_LINES - art->n_attributes_set - space_up;
  if(art->n_attributes_set > NUMBER_OF_LINES) {
    limit_up = 0;
    limit_down = art->n_attributes_set;
  }
  else {
    limit_up = space_up;
    limit_down = NUMBER_OF_LINES-space_down;
  }
  bool add_space = false;
  uint32_t len = max(art->n_attributes_set, NUMBER_OF_LINES);
  
  for(uint32_t n=0; n < len; n++) {
    if(n >= art->additional_spaces && n < NUMBER_OF_LINES + art->additional_spaces)
      callback_print_algorithm(art, n - art->additional_spaces);
    else
      printf("%*s", LINE_SIZE, "");

    if(n >= limit_up && n < limit_down) {
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

}

void print_ascii(struct ascii* art) {
  uint32_t longest_attribute = longest_attribute_length(art);
  
  if(art->vendor == SOC_VENDOR_SNAPDRAGON || art->vendor == SOC_VENDOR_MEDIATEK || art->vendor == SOC_VENDOR_KIRIN || art->vendor == SOC_VENDOR_BROADCOM)
    print_ascii_arm(art, longest_attribute, &print_algorithm_snapd_mtk);      
  else if(art->vendor == SOC_VENDOR_EXYNOS)
    print_ascii_arm(art, longest_attribute, &print_algorithm_samsung);      
  else {
    if(art->vendor != SOC_VENDOR_UNKNOWN)
      printWarn("Invalid SOC vendor: %d\n", art->vendor);
    print_ascii_arm(art, longest_attribute, &print_algorithm_arm);
  }
  
}

bool print_cpufetch_arm(struct cpuInfo* cpu, STYLE s, struct colors* cs) {   
  struct ascii* art = set_ascii(get_soc_vendor(cpu->soc), s, cs);
  if(art == NULL)
    return false;  
  
  char* manufacturing_process = get_str_process(cpu->soc);
  char* soc_name = get_soc_name(cpu->soc);
  char* features = get_str_features(cpu);
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
    if(features != NULL) {
      setAttribute(art, ATTRIBUTE_FEATURES, features);    
    }
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
      if(features != NULL) {
        setAttribute(art, ATTRIBUTE_FEATURES, features);    
      }
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
    art->additional_spaces = (art->n_attributes_set - NUMBER_OF_LINES) / 2;
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
  int len = sizeof(ASCII_ARRAY) / sizeof(ASCII_ARRAY[0]);
  for(int i=0; i < len; i++) {
    const char* ascii = ASCII_ARRAY[i];
    if(strlen(ascii) != (NUMBER_OF_LINES * LINE_SIZE)) {
      printBug("ASCII art %d is wrong! ASCII length: %d, expected length: %d", i, strlen(ascii), (NUMBER_OF_LINES * LINE_SIZE));
      return false;
    }
  }
  
#ifdef ARCH_X86
  return print_cpufetch_x86(cpu, s, cs);
#elif ARCH_ARM
  return print_cpufetch_arm(cpu, s, cs);  
#endif
}
