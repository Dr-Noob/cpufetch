#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>

#include "printer.h"
#include "ascii.h"
#include "../common/global.h"
#include "../common/cpu.h"

#ifdef ARCH_X86
  #include "../x86/uarch.h"
  #include "../x86/cpuid.h"
#elif ARCH_PPC
  #include "../ppc/uarch.h"
  #include "../ppc/ppc.h"
#else
  #include "../arm/uarch.h"
  #include "../arm/midr.h"
  #include "../arm/soc.h"
#endif

#ifdef _WIN32
  #define NOMINMAX
  #include <Windows.h>
#else
  #ifdef  __linux__
    #ifndef _POSIX_C_SOURCE
      #define _POSIX_C_SOURCE 199309L
    #endif
  #endif
  #include <sys/ioctl.h>
  #include <unistd.h>
#endif

#define max(a,b) (((a)>(b))?(a):(b))
#define MAX_ATTRIBUTES      100

enum {
#if defined(ARCH_X86) || defined(ARCH_PPC)
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
#elif ARCH_PPC
  ATTRIBUTE_ALTIVEC,
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
#if defined(ARCH_X86) || defined(ARCH_PPC)
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
#elif ARCH_PPC
  "Altivec: ",
#elif defined(ARCH_ARM)
  "Features: ",
#endif
  "L1i Size:",
  "L1d Size:",
  "L2 Size:",
  "L3 Size:",
  "Peak Performance:",
};

struct terminal {
  int w;
  int h;
};

struct attribute {
  int type;
  char* value;
};

struct ascii {
  struct ascii_logo* art;
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
  char* str = emalloc(sizeof(char) * 100);
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

struct ascii* set_ascii(VENDOR vendor, STYLE style) {
  struct ascii* art = emalloc(sizeof(struct ascii));

  art->n_attributes_set = 0;
  art->additional_spaces = 0;
  art->vendor = vendor;
  art->attributes = emalloc(sizeof(struct attribute *) * MAX_ATTRIBUTES);
  for(uint32_t i=0; i < MAX_ATTRIBUTES; i++) {
    art->attributes[i] = emalloc(sizeof(struct attribute));
    art->attributes[i]->type = 0;
    art->attributes[i]->value = NULL;
  }

  #ifdef _WIN32
    // Old Windows do not define the flag
    #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
      #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
    #endif

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

  return art;
}

void parse_print_color(struct ascii* art, uint32_t* logo_pos) {
  struct ascii_logo* logo = art->art;
  char color_id_str = logo->art[*logo_pos + 2];

  if(color_id_str == 'R') {
    printf("%s", art->reset);
  }
  else {
    int color_id = (color_id_str - '0') - 1;
    printf("%s", logo->color_ascii[color_id]);
  }

  *logo_pos += 3;
}

bool ascii_fits_screen(int termw, struct ascii_logo logo, int lf) {
  return termw - ((int) logo.width + lf) >= 0;
}

// TODO: Instead of using a function to do so, change ascii.h
// and store an color ID that is converted to BG or FG depending
// on logo->replace_blocks
void replace_bgbyfg_color(struct ascii_logo* logo) {
  // Replace background by foreground color
  for(int i=0; i < 2; i++) {
    if(logo->color_ascii[i] == NULL) break;

    if(strcmp(logo->color_ascii[i], COLOR_BG_BLACK) == 0) strcpy(logo->color_ascii[i], COLOR_FG_BLACK);
    else if(strcmp(logo->color_ascii[i], COLOR_BG_RED) == 0) strcpy(logo->color_ascii[i], COLOR_FG_RED);
    else if(strcmp(logo->color_ascii[i], COLOR_BG_GREEN) == 0) strcpy(logo->color_ascii[i], COLOR_FG_GREEN);
    else if(strcmp(logo->color_ascii[i], COLOR_BG_YELLOW) == 0) strcpy(logo->color_ascii[i], COLOR_FG_YELLOW);
    else if(strcmp(logo->color_ascii[i], COLOR_BG_BLUE) == 0) strcpy(logo->color_ascii[i], COLOR_FG_BLUE);
    else if(strcmp(logo->color_ascii[i], COLOR_BG_MAGENTA) == 0) strcpy(logo->color_ascii[i], COLOR_FG_MAGENTA);
    else if(strcmp(logo->color_ascii[i], COLOR_BG_CYAN) == 0) strcpy(logo->color_ascii[i], COLOR_FG_CYAN);
    else if(strcmp(logo->color_ascii[i], COLOR_BG_WHITE) == 0) strcpy(logo->color_ascii[i], COLOR_FG_WHITE);
  }
}

void choose_ascii_art(struct ascii* art, struct color** cs, struct terminal* term, int lf) {
  // 1. Choose logo
#ifdef ARCH_X86
  if(art->vendor == CPU_VENDOR_INTEL) {
    if(term != NULL && ascii_fits_screen(term->w, logo_intel_l, lf))
      art->art = &logo_intel_l;
    else
      art->art = &logo_intel;
  }
  else if(art->vendor == CPU_VENDOR_AMD) {
    if(term != NULL && ascii_fits_screen(term->w, logo_amd_l, lf))
      art->art = &logo_amd_l;
    else
      art->art = &logo_amd;
  }
  else {
    art->art = &logo_unknown;
  }
#elif ARCH_PPC
  if(term != NULL && ascii_fits_screen(term->w, logo_ibm_l, lf))
    art->art = &logo_ibm_l;
  else
    art->art = &logo_ibm;
#elif ARCH_ARM
  if(art->vendor == SOC_VENDOR_SNAPDRAGON)
    art->art = &logo_snapd;
  else if(art->vendor == SOC_VENDOR_MEDIATEK)
    art->art = &logo_mtk;
  else if(art->vendor == SOC_VENDOR_EXYNOS)
    art->art = &logo_exynos;
  else if(art->vendor == SOC_VENDOR_KIRIN)
    art->art = &logo_kirin;
  else if(art->vendor == SOC_VENDOR_BROADCOM)
    art->art = &logo_broadcom;
  else {
    if(term != NULL && ascii_fits_screen(term->w, logo_arm_l, lf))
      art->art = &logo_arm_l;
    else
      art->art = &logo_arm;
  }
#endif

  // 2. Choose colors
  struct ascii_logo* logo = art->art;

  switch(art->style) {
    case STYLE_LEGACY:
      logo->replace_blocks = false;
      strcpy(logo->color_text[0], COLOR_NONE);
      strcpy(logo->color_text[1], COLOR_NONE);
      strcpy(logo->color_ascii[0], COLOR_NONE);
      strcpy(logo->color_ascii[1], COLOR_NONE);
      art->reset[0] = '\0';
      break;
    case STYLE_RETRO:
      logo->replace_blocks = false;
      replace_bgbyfg_color(logo);
      // fall through
    case STYLE_FANCY:
      if(cs != NULL) {
        strcpy(logo->color_text[0], rgb_to_ansi(cs[2], false, true));
        strcpy(logo->color_text[1], rgb_to_ansi(cs[3], false, true));
        strcpy(logo->color_ascii[0], rgb_to_ansi(cs[0], true, true));
        strcpy(logo->color_ascii[1], rgb_to_ansi(cs[1], true, true));
      }
      strcpy(art->reset, COLOR_RESET);
      break;
    case STYLE_INVALID:
    default:
      printBug("Found invalid style (%d)", art->style);
  }
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

uint32_t longest_field_length(struct ascii* art, int la) {
  uint32_t max = 0;
  uint64_t len = 0;

  for(uint32_t i=0; i < art->n_attributes_set; i++) {
    if(art->attributes[i]->value != NULL) {
      // longest attribute + 1 (space) + longest value
      len = la + 1 + strlen(art->attributes[i]->value);

      if(len > max) max = len;
    }
  }

  return max;
}

#if defined(ARCH_X86) || defined(ARCH_PPC)
void print_ascii_generic(struct ascii* art, uint32_t la) {
  struct ascii_logo* logo = art->art;
  int attr_to_print = 0;
  int attr_type;
  char* attr_value;
  uint32_t space_right;
  int32_t space_up = ((int)logo->height - (int)art->n_attributes_set)/2;
  int32_t space_down = (int)logo->height - (int)art->n_attributes_set - (int)space_up;
  uint32_t logo_pos = 0;
  int32_t iters = max(logo->height, art->n_attributes_set);

  printf("\n");
  for(int32_t n=0; n < iters; n++) {
    // 1. Print logo
    if(space_up > 0 || (space_up + n >= 0 && space_up + n < (int)logo->height)) {
      for(uint32_t i=0; i < logo->width; i++) {
        if(logo->art[logo_pos] == '$') {
          if(logo->replace_blocks) logo_pos += 3;
          else parse_print_color(art, &logo_pos);
        }
        if(logo->replace_blocks && logo->art[logo_pos] != ' ') {
          if(logo->art[logo_pos] == '#') printf("%s%c%s", logo->color_ascii[0], ' ', art->reset);
          else if(logo->art[logo_pos] == '@') printf("%s%c%s", logo->color_ascii[1], ' ', art->reset);
          else printf("%c", logo->art[logo_pos]);
        }
        else
          printf("%c", logo->art[logo_pos]);

        logo_pos++;
      }
      printf("%s", art->reset);
    }
    else {
      // If logo should not be printed, fill with spaces
      printf("%*c", logo->width, ' ');
    }

    // 2. Print text
    if(space_up < 0 || (n > space_up-1 && n < (int)logo->height - space_down)) {
      attr_type = art->attributes[attr_to_print]->type;
      attr_value = art->attributes[attr_to_print]->value;
      attr_to_print++;

      space_right = 1 + (la - strlen(ATTRIBUTE_FIELDS[attr_type]));
      printf("%s%s%s%*s%s%s%s\n", logo->color_text[0], ATTRIBUTE_FIELDS[attr_type], art->reset, space_right, "", logo->color_text[1], attr_value, art->reset);
    }
    else printf("\n");
  }
  printf("\n");
}
#endif

#ifdef ARCH_X86
bool print_cpufetch_x86(struct cpuInfo* cpu, STYLE s, struct color** cs, struct terminal* term) {
  struct ascii* art = set_ascii(get_cpu_vendor(cpu), s);
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
  char* pp = get_str_peak_performance(cpu->peak_performance);

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

  uint32_t longest_attribute = longest_attribute_length(art);
  uint32_t longest_field = longest_field_length(art, longest_attribute);
  choose_ascii_art(art, cs, term, longest_field);

  print_ascii_generic(art, longest_attribute);

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

#ifdef ARCH_PPC
bool print_cpufetch_ppc(struct cpuInfo* cpu, STYLE s, struct color** cs, struct terminal* term) {
  struct ascii* art = set_ascii(get_cpu_vendor(cpu), s);
  if(art == NULL)
    return false;

  char* uarch = get_str_uarch(cpu);
  char* manufacturing_process = get_str_process(cpu);
  char* sockets = get_str_sockets(cpu->topo);
  char* max_frequency = get_str_freq(cpu->freq);
  char* cpu_name = get_str_cpu_name(cpu);
  char* n_cores = get_str_topology(cpu->topo, false);
  char* n_cores_dual = get_str_topology(cpu->topo, true);
  char* altivec = get_str_altivec(cpu);

  char* l1i = get_str_l1i(cpu->cach);
  char* l1d = get_str_l1d(cpu->cach);
  char* l2 = get_str_l2(cpu->cach);
  char* l3 = get_str_l3(cpu->cach);
  char* pp = get_str_peak_performance(cpu->peak_performance);

  if(cpu_name != NULL) {
    setAttribute(art,ATTRIBUTE_NAME,cpu_name);
  }
  setAttribute(art,ATTRIBUTE_UARCH,uarch);
  setAttribute(art,ATTRIBUTE_TECHNOLOGY,manufacturing_process);
  setAttribute(art,ATTRIBUTE_FREQUENCY,max_frequency);
  uint32_t socket_num = get_nsockets(cpu->topo);
  if (socket_num > 1) {
    setAttribute(art, ATTRIBUTE_SOCKETS, sockets);
    setAttribute(art, ATTRIBUTE_NCORES, n_cores);
    setAttribute(art, ATTRIBUTE_NCORES_DUAL, n_cores_dual);
  }
  else {
    setAttribute(art,ATTRIBUTE_NCORES, n_cores);
  }
  setAttribute(art,ATTRIBUTE_ALTIVEC, altivec);
  setAttribute(art,ATTRIBUTE_L1i,l1i);
  setAttribute(art,ATTRIBUTE_L1d,l1d);
  setAttribute(art,ATTRIBUTE_L2,l2);
  if(l3 != NULL) {
    setAttribute(art,ATTRIBUTE_L3,l3);
  }
  setAttribute(art,ATTRIBUTE_PEAK,pp);

  uint32_t longest_attribute = longest_attribute_length(art);
  uint32_t longest_field = longest_field_length(art, longest_attribute);
  choose_ascii_art(art, cs, term, longest_field);

  print_ascii_generic(art, longest_attribute);

  return true;
}
#endif

#ifdef ARCH_ARM
uint32_t longest_field_length_arm(struct ascii* art, int la) {
  uint32_t max = 0;
  uint64_t len = 0;

  for(uint32_t i=0; i < art->n_attributes_set; i++) {
    if(art->attributes[i]->value != NULL) {
      // longest attribute + 1 (space) + longest value
      len = la + 1 + strlen(art->attributes[i]->value);
      if(art->attributes[i]->type == ATTRIBUTE_UARCH     ||
         art->attributes[i]->type == ATTRIBUTE_FREQUENCY ||
         art->attributes[i]->type == ATTRIBUTE_NCORES    ||
         art->attributes[i]->type == ATTRIBUTE_FEATURES) {
        len += 2;
      }
      if(len > max) max = len;
    }
  }

  return max;
}

void print_ascii_arm(struct ascii* art, uint32_t la) {
  struct ascii_logo* logo = art->art;
  int attr_to_print = 0;
  int attr_type;
  char* attr_value;
  int32_t limit_up;
  int32_t limit_down;
  uint32_t logo_pos = 0;
  uint32_t space_right;
  int32_t space_up = ((int)logo->height - (int)art->n_attributes_set)/2;
  int32_t space_down = (int)logo->height - (int)art->n_attributes_set - (int)space_up;

  if(art->n_attributes_set > logo->height) {
    limit_up = 0;
    limit_down = art->n_attributes_set;
  }
  else {
    limit_up = space_up;
    limit_down = logo->height - space_down;
  }
  bool add_space = false;
  int32_t iters = max(logo->height, art->n_attributes_set);

  for(int32_t n=0; n < iters; n++) {
    // 1. Print logo
    if(n >= (int) art->additional_spaces && n < (int) logo->height + (int) art->additional_spaces) {
      for(uint32_t i=0; i < logo->width; i++) {
        if(logo->art[logo_pos] == '$') {
          if(logo->replace_blocks) logo_pos += 3;
          else parse_print_color(art, &logo_pos);
        }
        if(logo->replace_blocks && logo->art[logo_pos] != ' ') {
          if(logo->art[logo_pos] == '#') printf("%s%c%s", logo->color_ascii[0], ' ', art->reset);
          else if(logo->art[logo_pos] == '@') printf("%s%c%s", logo->color_ascii[1], ' ', art->reset);
          else printf("%c", logo->art[logo_pos]);
        }
        else
          printf("%c", logo->art[logo_pos]);

        logo_pos++;
      }
      printf("%s", art->reset);
    }
    else {
      // If logo should not be printed, fill with spaces
      printf("%*c", logo->width, ' ');
    }

    // 2. Print text
    if(n >= limit_up && n < limit_down) {
      attr_type = art->attributes[attr_to_print]->type;
      attr_value = art->attributes[attr_to_print]->value;
      attr_to_print++;

      if(attr_type == ATTRIBUTE_PEAK) {
        add_space = false;
      }
      if(attr_type == ATTRIBUTE_CPU_NUM) {
        printf("%s%s%s\n", logo->color_text[0], attr_value, art->reset);
        add_space = true;
      }
      else {
        if(add_space) {
          space_right = 1 + (la - strlen(ATTRIBUTE_FIELDS[attr_type]));
          printf("  %s%s%s%*s%s%s%s\n", logo->color_text[0], ATTRIBUTE_FIELDS[attr_type], art->reset, space_right, "", logo->color_text[1], attr_value, art->reset);
        }
        else {
          space_right = 2 + 1 + (la - strlen(ATTRIBUTE_FIELDS[attr_type]));
          printf("%s%s%s%*s%s%s%s\n", logo->color_text[0], ATTRIBUTE_FIELDS[attr_type], art->reset, space_right, "", logo->color_text[1], attr_value, art->reset);
        }
      }
    }
    else printf("\n");
  }

}

bool print_cpufetch_arm(struct cpuInfo* cpu, STYLE s, struct color** cs, struct terminal* term) {
  struct ascii* art = set_ascii(get_soc_vendor(cpu->soc), s);
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
    /*
     * char* l1i = get_str_l1i(cpu->cach);
     * char* l1d = get_str_l1d(cpu->cach);
     * char* l2 = get_str_l2(cpu->cach);
     * char* l3 = get_str_l3(cpu->cach);
     * Do not setAttribute for caches.
     * Cache functionality may be implemented
     * in the future
    */

    setAttribute(art,ATTRIBUTE_UARCH,uarch);
    setAttribute(art,ATTRIBUTE_FREQUENCY,max_frequency);
    setAttribute(art,ATTRIBUTE_NCORES,n_cores);
    if(features != NULL) {
      setAttribute(art, ATTRIBUTE_FEATURES, features);
    }
  }
  else {
    struct cpuInfo* ptr = cpu;
    for(int i = 0; i < cpu->num_cpus; ptr = ptr->next_cpu, i++) {
      char* uarch = get_str_uarch(ptr);
      char* max_frequency = get_str_freq(ptr->freq);
      char* n_cores = get_str_topology(ptr, ptr->topo, false);
      /*
       * char* l1i = get_str_l1i(cpu->cach);
       * char* l1d = get_str_l1d(cpu->cach);
       * char* l2 = get_str_l2(cpu->cach);
       * char* l3 = get_str_l3(cpu->cach);
       * Do not setAttribute for caches.
       * Cache functionality may be implemented
       * in the future
       */

      char* cpu_num = emalloc(sizeof(char) * 9);
      sprintf(cpu_num, "CPU %d:", i+1);
      setAttribute(art, ATTRIBUTE_CPU_NUM, cpu_num);
      setAttribute(art, ATTRIBUTE_UARCH, uarch);
      setAttribute(art, ATTRIBUTE_FREQUENCY, max_frequency);
      setAttribute(art, ATTRIBUTE_NCORES, n_cores);
      if(features != NULL) {
        setAttribute(art, ATTRIBUTE_FEATURES, features);
      }
    }
  }
  char* pp = get_str_peak_performance(cpu->peak_performance);
  setAttribute(art,ATTRIBUTE_PEAK,pp);
  if(cpu->hv->present) {
    setAttribute(art, ATTRIBUTE_HYPERVISOR, cpu->hv->hv_name);
  }

  uint32_t longest_attribute = longest_attribute_length(art);
  uint32_t longest_field = longest_field_length_arm(art, longest_attribute);
  choose_ascii_art(art, cs, term, longest_field);

  struct ascii_logo* logo = art->art;
  if(art->n_attributes_set > logo->height) {
    art->additional_spaces = (art->n_attributes_set - logo->height) / 2;
  }

  print_ascii_arm(art, longest_attribute);

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

struct terminal* get_terminal_size() {
  struct terminal* term = emalloc(sizeof(struct terminal));

#ifdef _WIN32
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if(GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi) == 0) {
    printWarn("GetConsoleScreenBufferInfo failed");
    return NULL;
  }
  term->w = csbi.srWindow.Right - csbi.srWindow.Left + 1;
  term->h = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
  struct winsize w;
  if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
    printErr("ioctl: %s", strerror(errno));
    return NULL;
  }
  term->h = w.ws_row;
  term->w = w.ws_col;
#endif

  return term;
}

bool print_cpufetch(struct cpuInfo* cpu, STYLE s, struct color** cs) {
  struct terminal* term = get_terminal_size();

#ifdef ARCH_X86
  return print_cpufetch_x86(cpu, s, cs, term);
#elif ARCH_PPC
  return print_cpufetch_ppc(cpu, s, cs, term);
#elif ARCH_ARM
  return print_cpufetch_arm(cpu, s, cs, term);
#endif
}
