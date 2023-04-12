#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <stdarg.h>

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
#elif ARCH_ARM
  #include "../arm/uarch.h"
  #include "../arm/midr.h"
  #include "../arm/soc.h"
#elif ARCH_RISCV
  #include "../riscv/riscv.h"
  #include "../riscv/uarch.h"
  #include "../riscv/soc.h"
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

#define LINE_BUFFER_SIZE    (1<<16)
#define MAX_ATTRIBUTES      100
#define MAX_TERM_SIZE       1024

enum {
#if defined(ARCH_X86) || defined(ARCH_PPC)
  ATTRIBUTE_NAME,
#elif defined(ARCH_ARM) || defined(ARCH_RISCV)
  ATTRIBUTE_SOC,
#endif
#if defined(ARCH_X86) || defined(ARCH_ARM)
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
#ifdef ARCH_X86
  "Name:",
#elif ARCH_PPC
  "Part Number:",
#elif defined(ARCH_ARM) || defined(ARCH_RISCV)
  "SoC:",
#endif
#if defined(ARCH_X86) || defined(ARCH_ARM)
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

static const char* ATTRIBUTE_FIELDS_SHORT [] = {
#if defined(ARCH_X86)
  "Name:",
#elif ARCH_PPC
  "P/N:",
#elif ARCH_ARM
  "SoC:",
#endif
#if defined(ARCH_X86) || defined(ARCH_ARM)
  "",
#endif
  "Hypervisor:",
  "uArch:",
  "Technology:",
  "Max Freq:",
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
  "Peak Perf.:",
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
  bool new_intel_logo;
  VENDOR vendor;
  STYLE style;
};

struct line_buffer {
  char* buf;
  int pos;
  int chars;
};

// Writes to the line buffer the output passed in fmt
void printOut(struct line_buffer* lbuf, int chars, const char *fmt, ...) {
  int buffer_size = 4096;
  char buffer[buffer_size];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer,buffer_size, fmt, args);
  va_end(args);

  if(lbuf->pos > LINE_BUFFER_SIZE) {
    printBug("Line buffer size exceeded. Max is %d, current position is %d", lbuf->pos, LINE_BUFFER_SIZE);
  }
  else {
    lbuf->pos += sprintf(lbuf->buf + lbuf->pos, "%s", buffer);
    lbuf->chars += chars;
  }
}

// Writes a full line (restricting the output length) using the line buffer
void printOutLine(struct line_buffer* lbuf, struct ascii* art, int termw) {
  int chars_to_print = min(lbuf->chars, termw);
  int pos = 0;

  for(int i=0; i < chars_to_print; i++) {
    while(lbuf->buf[pos] == '\x1b') {
      // Skip color
      while(lbuf->buf[pos] != 'm') {
        printf("%c", lbuf->buf[pos]);
        pos++;
      }
      printf("%c", lbuf->buf[pos]);
      pos++;
    }

    printf("%c", lbuf->buf[pos]);
    pos++;
  }

  // Make sure weset the color
  printf("%s", art->reset);

  lbuf->pos = 0;
  lbuf->chars = 0;
}

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

void parse_print_color(struct ascii* art, struct line_buffer* lbuf, uint32_t* logo_pos) {
  struct ascii_logo* logo = art->art;
  char color_id_str = logo->art[*logo_pos + 2];

  if(color_id_str == 'R') {
    printOut(lbuf, 0, "%s", art->reset);
  }
  else {
    int color_id = (color_id_str - '0') - 1;
    printOut(lbuf, 0, "%s", logo->color_ascii[color_id]);
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
  for(int i=0; i < 3; i++) {
    if(strcmp(logo->color_ascii[i], C_BG_BLACK) == 0) strcpy(logo->color_ascii[i], C_FG_BLACK);
    else if(strcmp(logo->color_ascii[i], C_BG_RED) == 0) strcpy(logo->color_ascii[i], C_FG_RED);
    else if(strcmp(logo->color_ascii[i], C_BG_GREEN) == 0) strcpy(logo->color_ascii[i], C_FG_GREEN);
    else if(strcmp(logo->color_ascii[i], C_BG_YELLOW) == 0) strcpy(logo->color_ascii[i], C_FG_YELLOW);
    else if(strcmp(logo->color_ascii[i], C_BG_BLUE) == 0) strcpy(logo->color_ascii[i], C_FG_BLUE);
    else if(strcmp(logo->color_ascii[i], C_BG_MAGENTA) == 0) strcpy(logo->color_ascii[i], C_FG_MAGENTA);
    else if(strcmp(logo->color_ascii[i], C_BG_CYAN) == 0) strcpy(logo->color_ascii[i], C_FG_CYAN);
    else if(strcmp(logo->color_ascii[i], C_BG_WHITE) == 0) strcpy(logo->color_ascii[i], C_FG_WHITE);
  }
}

struct ascii_logo* choose_ascii_art_aux(struct ascii_logo* logo_long, struct ascii_logo* logo_short, struct terminal* term, int lf) {
  if(show_logo_long()) return logo_long;
  if(show_logo_short()) return logo_short;
  if(ascii_fits_screen(term->w, *logo_long, lf)) {
    return logo_long;
  }
  else {
    return logo_short;
  }
}

void choose_ascii_art(struct ascii* art, struct color** cs, struct terminal* term, int lf) {
  // 1. Choose logo
#ifdef ARCH_X86
  if(art->vendor == CPU_VENDOR_INTEL) {
    if(art->new_intel_logo) {
      art->art = choose_ascii_art_aux(&logo_intel_l_new, &logo_intel_new, term, lf);
    }
    else {
      art->art = choose_ascii_art_aux(&logo_intel_l, &logo_intel, term, lf);
    }
  }
  else if(art->vendor == CPU_VENDOR_AMD) {
    art->art = choose_ascii_art_aux(&logo_amd_l, &logo_amd, term, lf);
  }
  else {
    art->art = &logo_unknown;
  }
#elif ARCH_PPC
  art->art = choose_ascii_art_aux(&logo_ibm_l, &logo_ibm, term, lf);
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
  else if(art->vendor == SOC_VENDOR_APPLE)
    art->art = &logo_apple;
  else if(art->vendor == SOC_VENDOR_ALLWINNER)
    art->art = &logo_allwinner;
  else if(art->vendor == SOC_VENDOR_ROCKCHIP)
    art->art = &logo_rockchip;
  else {
    art->art = choose_ascii_art_aux(&logo_arm_l, &logo_arm, term, lf);
  }
#elif ARCH_RISCV
  if(art->vendor == SOC_VENDOR_SIFIVE)
    art->art = choose_ascii_art_aux(&logo_sifive_l, &logo_sifive, term, lf);
  else if(art->vendor == SOC_VENDOR_STARFIVE)
    art->art = choose_ascii_art_aux(&logo_starfive_l, &logo_starfive, term, lf);
  else if(art->vendor == SOC_VENDOR_ALLWINNER)
    art->art = &logo_allwinner;
  else
    art->art = &logo_riscv;
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
      strcpy(logo->color_ascii[2], COLOR_NONE);
      art->reset[0] = '\0';
      break;
    case STYLE_RETRO:
      logo->replace_blocks = false;
      replace_bgbyfg_color(logo);
      // fall through
    case STYLE_FANCY:
      if(cs != NULL) {
        strcpy(logo->color_text[0], rgb_to_ansi(cs[3], false, true));
        strcpy(logo->color_text[1], rgb_to_ansi(cs[4], false, true));
        strcpy(logo->color_ascii[0], rgb_to_ansi(cs[0], logo->replace_blocks, true));
        strcpy(logo->color_ascii[1], rgb_to_ansi(cs[1], logo->replace_blocks, true));
        strcpy(logo->color_ascii[2], rgb_to_ansi(cs[2], logo->replace_blocks, true));
      }
      strcpy(art->reset, COLOR_RESET);
      break;
    case STYLE_INVALID:
    default:
      printBug("Found invalid style (%d)", art->style);
  }
}

uint32_t longest_attribute_length(struct ascii* art, const char** attribute_fields) {
  uint32_t max = 0;
  uint64_t len = 0;

  for(uint32_t i=0; i < art->n_attributes_set; i++) {
    if(art->attributes[i]->value != NULL) {
      len = strlen(attribute_fields[art->attributes[i]->type]);
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

#if defined(ARCH_X86) || defined(ARCH_PPC) || defined(ARCH_RISCV)
void print_ascii_generic(struct ascii* art, uint32_t la, int32_t termw, const char** attribute_fields, bool hybrid_architecture) {
  struct ascii_logo* logo = art->art;
  int attr_to_print = 0;
  int attr_type;
  char* attr_value;
  int32_t beg_space;
  int32_t space_right;
  int32_t space_up = ((int)logo->height - (int)art->n_attributes_set)/2;
  int32_t space_down = (int)logo->height - (int)art->n_attributes_set - (int)space_up;
  uint32_t logo_pos = 0;
  int32_t iters = max(logo->height, art->n_attributes_set);

  struct line_buffer* lbuf = emalloc(sizeof(struct line_buffer));
  lbuf->buf = emalloc(sizeof(char) * LINE_BUFFER_SIZE);
  lbuf->pos = 0;
  lbuf->chars = 0;
  bool add_space = false;

  printf("\n");
  for(int32_t n=0; n < iters; n++) {
    // 1. Print logo
    if(space_up > 0 || (space_up + n >= 0 && space_up + n < (int)logo->height)) {
      for(uint32_t i=0; i < logo->width; i++) {
        if(logo->art[logo_pos] == '$') {
          if(logo->replace_blocks) logo_pos += 3;
          else parse_print_color(art, lbuf, &logo_pos);
        }
        if(logo->replace_blocks && logo->art[logo_pos] != ' ') {
          if(logo->art[logo_pos] == '#') printOut(lbuf, 1, "%s%c%s", logo->color_ascii[0], ' ', art->reset);
          else if(logo->art[logo_pos] == '@') printOut(lbuf, 1, "%s%c%s", logo->color_ascii[1], ' ', art->reset);
          else if(logo->art[logo_pos] == '%') printOut(lbuf, 1, "%s%c%s", logo->color_ascii[2], ' ', art->reset);
          else printOut(lbuf, 1, "%c", logo->art[logo_pos]);
        }
        else
          printOut(lbuf, 1, "%c", logo->art[logo_pos]);

        logo_pos++;
      }
      printOut(lbuf, 0, "%s", art->reset);
    }
    else {
      // If logo should not be printed, fill with spaces
      printOut(lbuf, logo->width, "%*c", logo->width, ' ');
    }

    // 2. Print text
    if(space_up < 0 || (n > space_up-1 && n < (int)logo->height - space_down)) {
      attr_type = art->attributes[attr_to_print]->type;
      attr_value = art->attributes[attr_to_print]->value;
      attr_to_print++;

#ifdef ARCH_X86
      if(attr_type == ATTRIBUTE_L3) {
        add_space = false;
      }
      if(attr_type == ATTRIBUTE_CPU_NUM) {
        printOut(lbuf, strlen(attr_value), "%s%s%s", logo->color_text[0], attr_value, art->reset);
        add_space = true;
      }
      else {
#endif
        beg_space = 0;
        space_right = 2 + 1 + (la - strlen(attribute_fields[attr_type]));
        if(hybrid_architecture && add_space) {
          beg_space = 2;
          space_right -= 2;
        }

        printOut(lbuf, beg_space + strlen(attribute_fields[attr_type]) + space_right + strlen(attr_value),
                 "%*s%s%s%s%*s%s%s%s", beg_space, "", logo->color_text[0], attribute_fields[attr_type], art->reset, space_right, "", logo->color_text[1], attr_value, art->reset);
#ifdef ARCH_X86
      }
#endif
    }
    printOutLine(lbuf, art, termw);
    printf("\n");
  }
  printf("\n");

  free(lbuf->buf);
  free(lbuf);
}
#endif

#ifdef ARCH_X86
bool choose_new_intel_logo(struct cpuInfo* cpu) {
  if(show_logo_intel_new()) return true;
  if(show_logo_intel_old()) return false;
  return choose_new_intel_logo_uarch(cpu);
}

bool print_cpufetch_x86(struct cpuInfo* cpu, STYLE s, struct color** cs, struct terminal* term, bool fcpuname) {
  struct ascii* art = set_ascii(get_cpu_vendor(cpu), s);
  if(art == NULL)
    return false;

  art->new_intel_logo = choose_new_intel_logo(cpu);

  uint32_t socket_num = 1;
  char* l1i, *l1d, *l2, *l3, *n_cores, *n_cores_dual, *sockets;
  l1i = l1d = l2 = l3 = n_cores = n_cores_dual = sockets = NULL;

  char* cpu_name = get_str_cpu_name(cpu, fcpuname);
  char* uarch = get_str_uarch(cpu);
  char* pp = get_str_peak_performance(cpu->peak_performance);
  char* manufacturing_process = get_str_process(cpu);
  bool hybrid_architecture = cpu->next_cpu != NULL;

  if(cpu->cach != NULL) {
    l3 = get_str_l3(cpu->cach);
  }

  setAttribute(art, ATTRIBUTE_NAME, cpu_name);
  if(cpu->hv->present) {
    setAttribute(art, ATTRIBUTE_HYPERVISOR, cpu->hv->hv_name);
  }
  setAttribute(art, ATTRIBUTE_UARCH, uarch);
  setAttribute(art, ATTRIBUTE_TECHNOLOGY, manufacturing_process);

  struct cpuInfo* ptr = cpu;
  for(int i = 0; i < cpu->num_cpus; ptr = ptr->next_cpu, i++) {
    char* max_frequency = get_str_freq(ptr->freq);
    char* avx = get_str_avx(ptr);
    char* fma = get_str_fma(ptr);
    char* cpu_num = emalloc(sizeof(char) * 9);

    if(ptr->topo != NULL) {
      sockets = get_str_sockets(ptr->topo);
      n_cores = get_str_topology(ptr, ptr->topo, false);
      n_cores_dual = get_str_topology(ptr, ptr->topo, true);
    }

    if(ptr->cach != NULL) {
      l1i = get_str_l1i(ptr->cach);
      l1d = get_str_l1d(ptr->cach);
      l2 = get_str_l2(ptr->cach);
    }

    if(hybrid_architecture) {
      if(ptr->core_type == CORE_TYPE_EFFICIENCY) sprintf(cpu_num, "E-cores:");
      else if(ptr->core_type == CORE_TYPE_PERFORMANCE) sprintf(cpu_num, "P-cores:");
      else printBug("Found invalid core type!\n");

      setAttribute(art, ATTRIBUTE_CPU_NUM, cpu_num);
    }
    setAttribute(art, ATTRIBUTE_FREQUENCY, max_frequency);
    if(ptr->topo != NULL) {
      socket_num = get_nsockets(ptr->topo);
      if (socket_num > 1) {
        setAttribute(art, ATTRIBUTE_SOCKETS, sockets);
        setAttribute(art, ATTRIBUTE_NCORES, n_cores);
        setAttribute(art, ATTRIBUTE_NCORES_DUAL, n_cores_dual);
      }
      else {
        setAttribute(art, ATTRIBUTE_NCORES, n_cores);
      }
    }
    setAttribute(art, ATTRIBUTE_AVX, avx);
    setAttribute(art, ATTRIBUTE_FMA, fma);
    if(l1i != NULL) setAttribute(art, ATTRIBUTE_L1i, l1i);
    if(l1d != NULL) setAttribute(art, ATTRIBUTE_L1d, l1d);
    if(l2 != NULL) setAttribute(art, ATTRIBUTE_L2, l2);
  }
  if(l3 != NULL) setAttribute(art, ATTRIBUTE_L3, l3);
  setAttribute(art, ATTRIBUTE_PEAK, pp);

  // Step 3. Print output
  const char** attribute_fields = ATTRIBUTE_FIELDS;
  uint32_t longest_attribute = longest_attribute_length(art, attribute_fields);
  uint32_t longest_field = longest_field_length(art, longest_attribute);
  choose_ascii_art(art, cs, term, longest_field);

  if(!ascii_fits_screen(term->w, *art->art, longest_field)) {
    // Despite of choosing the smallest logo, the output does not fit
    // Choose the shorter field names and recalculate the longest attr
    attribute_fields = ATTRIBUTE_FIELDS_SHORT;
    longest_attribute = longest_attribute_length(art, attribute_fields);
  }

  print_ascii_generic(art, longest_attribute, term->w, attribute_fields, hybrid_architecture);

  free(manufacturing_process);
  free(sockets);
  free(n_cores);
  free(n_cores_dual);
  free(l1i);
  free(l1d);
  free(l2);
  free(l3);
  free(pp);

  free(art->attributes);
  free(art);

  if(cs != NULL) free_colors_struct(cs);
  if(cpu->cach != NULL) free_cache_struct(cpu->cach);
  if(cpu->topo != NULL) free_topo_struct(cpu->topo);
  free_freq_struct(cpu->freq);
  free_cpuinfo_struct(cpu);

  return true;
}
#endif

#ifdef ARCH_PPC
bool print_cpufetch_ppc(struct cpuInfo* cpu, STYLE s, struct color** cs, struct terminal* term, bool fcpuname) {
  struct ascii* art = set_ascii(get_cpu_vendor(cpu), s);
  if(art == NULL)
    return false;

  // Step 1. Retrieve attributes
  char* uarch = get_str_uarch(cpu);
  char* manufacturing_process = get_str_process(cpu);
  char* sockets = get_str_sockets(cpu->topo);
  char* max_frequency = get_str_freq(cpu->freq);
  char* cpu_name = get_str_cpu_name(cpu, fcpuname);
  char* n_cores = get_str_topology(cpu->topo, false);
  char* n_cores_dual = get_str_topology(cpu->topo, true);
  char* altivec = get_str_altivec(cpu);

  char* l1i = get_str_l1i(cpu->cach);
  char* l1d = get_str_l1d(cpu->cach);
  char* l2 = get_str_l2(cpu->cach);
  char* l3 = get_str_l3(cpu->cach);
  char* pp = get_str_peak_performance(cpu->peak_performance);

  // Step 2. Set attributes
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

  // Step 3. Print output
  const char** attribute_fields = ATTRIBUTE_FIELDS;
  uint32_t longest_attribute = longest_attribute_length(art, attribute_fields);
  uint32_t longest_field = longest_field_length(art, longest_attribute);
  choose_ascii_art(art, cs, term, longest_field);

  if(!ascii_fits_screen(term->w, *art->art, longest_field)) {
    // Despite of choosing the smallest logo, the output does not fit
    // Choose the shorter field names and recalculate the longest attr
    attribute_fields = ATTRIBUTE_FIELDS_SHORT;
    longest_attribute = longest_attribute_length(art, attribute_fields);
  }

  print_ascii_generic(art, longest_attribute, term->w, attribute_fields, false);

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

void print_ascii_arm(struct ascii* art, uint32_t la, int32_t termw, const char** attribute_fields) {
  struct ascii_logo* logo = art->art;
  int attr_to_print = 0;
  int attr_type;
  char* attr_value;
  int32_t beg_space;
  int32_t limit_up;
  int32_t limit_down;
  uint32_t logo_pos = 0;
  uint32_t space_right;
  int32_t space_up = ((int)logo->height - (int)art->n_attributes_set)/2;
  int32_t space_down = (int)logo->height - (int)art->n_attributes_set - (int)space_up;

  struct line_buffer* lbuf = emalloc(sizeof(struct line_buffer));
  lbuf->buf = emalloc(sizeof(char) * LINE_BUFFER_SIZE);
  lbuf->pos = 0;
  lbuf->chars = 0;

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

  printf("\n");
  for(int32_t n=0; n < iters; n++) {
    // 1. Print logo
    if(n >= (int) art->additional_spaces && n < (int) logo->height + (int) art->additional_spaces) {
      for(uint32_t i=0; i < logo->width; i++) {
        if(logo->art[logo_pos] == '$') {
          if(logo->replace_blocks) logo_pos += 3;
          else parse_print_color(art, lbuf, &logo_pos);
        }
        if(logo->replace_blocks && logo->art[logo_pos] != ' ') {
          if(logo->art[logo_pos] == '#') printOut(lbuf, 1, "%s%c%s", logo->color_ascii[0], ' ', art->reset);
          else if(logo->art[logo_pos] == '@') printOut(lbuf, 1, "%s%c%s", logo->color_ascii[1], ' ', art->reset);
          else if(logo->art[logo_pos] == '%') printOut(lbuf, 1, "%s%c%s", logo->color_ascii[2], ' ', art->reset);
          else printOut(lbuf, 1, "%c", logo->art[logo_pos]);
        }
        else
          printOut(lbuf, 1, "%c", logo->art[logo_pos]);

        logo_pos++;
      }
      printOut(lbuf, 0, "%s", art->reset);
    }
    else {
      // If logo should not be printed, fill with spaces
      printOut(lbuf, logo->width, "%*c", logo->width, ' ');
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
        printOut(lbuf, strlen(attr_value), "%s%s%s", logo->color_text[0], attr_value, art->reset);
        add_space = true;
      }
      else {
        beg_space = 0;
        space_right = 2 + 1 + (la - strlen(attribute_fields[attr_type]));
        if(add_space) {
          beg_space = 2;
          space_right -= 2;
        }

        printOut(lbuf, beg_space + strlen(attribute_fields[attr_type]) + space_right + strlen(attr_value),
               "%*s%s%s%s%*s%s%s%s", beg_space, "", logo->color_text[0], attribute_fields[attr_type], art->reset, space_right, "", logo->color_text[1], attr_value, art->reset);
      }
    }
    printOutLine(lbuf, art, termw);
    printf("\n");
  }
  printf("\n");

  free(lbuf->buf);
  free(lbuf);
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

  const char** attribute_fields = ATTRIBUTE_FIELDS;
  uint32_t longest_attribute = longest_attribute_length(art, attribute_fields);
  uint32_t longest_field = longest_field_length_arm(art, longest_attribute);
  choose_ascii_art(art, cs, term, longest_field);

  struct ascii_logo* logo = art->art;
  if(art->n_attributes_set > logo->height) {
    art->additional_spaces = (art->n_attributes_set - logo->height) / 2;
  }

  if(!ascii_fits_screen(term->w, *art->art, longest_field)) {
    // Despite of choosing the smallest logo, the output does not fit
    // Choose the shorter field names and recalculate the longest attr
    attribute_fields = ATTRIBUTE_FIELDS_SHORT;
    longest_attribute = longest_attribute_length(art, attribute_fields);
  }

  print_ascii_arm(art, longest_attribute, term->w, attribute_fields);

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

#ifdef ARCH_RISCV
bool print_cpufetch_riscv(struct cpuInfo* cpu, STYLE s, struct color** cs, struct terminal* term) {
  struct ascii* art = set_ascii(get_soc_vendor(cpu->soc), s);
  if(art == NULL)
    return false;

  // Step 1. Retrieve attributes
  char* uarch = get_str_uarch(cpu);
  char* manufacturing_process = get_str_process(cpu->soc);
  char* soc_name = get_soc_name(cpu->soc);
  char* extensions = get_str_extensions(cpu);
  char* max_frequency = get_str_freq(cpu->freq);
  char* n_cores = get_str_topology(cpu, cpu->topo);

  /*char* l1i = get_str_l1i(cpu->cach);
  char* l1d = get_str_l1d(cpu->cach);
  char* l2 = get_str_l2(cpu->cach);
  char* l3 = get_str_l3(cpu->cach);*/
  char* pp = get_str_peak_performance(cpu->peak_performance);

  // Step 2. Set attributes
  setAttribute(art,ATTRIBUTE_SOC,soc_name);
  setAttribute(art,ATTRIBUTE_TECHNOLOGY,manufacturing_process);
  setAttribute(art,ATTRIBUTE_UARCH,uarch);
  setAttribute(art,ATTRIBUTE_NCORES, n_cores);
  setAttribute(art,ATTRIBUTE_FREQUENCY,max_frequency);
  /*setAttribute(art,ATTRIBUTE_L1i,l1i);
  setAttribute(art,ATTRIBUTE_L1d,l1d);
  setAttribute(art,ATTRIBUTE_L2,l2);
  if(l3 != NULL) {
    setAttribute(art,ATTRIBUTE_L3,l3);
  }*/
  setAttribute(art,ATTRIBUTE_PEAK,pp);

  // Step 3. Print output
  const char** attribute_fields = ATTRIBUTE_FIELDS;
  uint32_t longest_attribute = longest_attribute_length(art, attribute_fields);
  uint32_t longest_field = longest_field_length(art, longest_attribute);
  choose_ascii_art(art, cs, term, longest_field);

  if(!ascii_fits_screen(term->w, *art->art, longest_field)) {
    // Despite of choosing the smallest logo, the output does not fit
    // Choose the shorter field names and recalculate the longest attr
    attribute_fields = ATTRIBUTE_FIELDS_SHORT;
    longest_attribute = longest_attribute_length(art, attribute_fields);
  }

  print_ascii_generic(art, longest_attribute, term->w, attribute_fields, false);

  return true;
}
#endif

struct terminal* get_terminal_size(void) {
  struct terminal* term = emalloc(sizeof(struct terminal));

#ifdef _WIN32
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if(GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi) == 0) {
    printWarn("get_terminal_size: GetConsoleScreenBufferInfo failed");
    term->w = MAX_TERM_SIZE;
    term->h = MAX_TERM_SIZE;
    return term;
  }
  term->w = csbi.srWindow.Right - csbi.srWindow.Left + 1;
  term->h = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
  struct winsize w;
  if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
    printWarn("get_terminal_size: ioctl: %s", strerror(errno));
    term->w = MAX_TERM_SIZE;
    term->h = MAX_TERM_SIZE;
    return term;
  }
  term->h = w.ws_row;
  term->w = w.ws_col;
#endif

  return term;
}

bool print_cpufetch(struct cpuInfo* cpu, STYLE s, struct color** cs, bool show_full_cpu_name) {
  struct terminal* term = get_terminal_size();

#ifdef ARCH_X86
  return print_cpufetch_x86(cpu, s, cs, term, show_full_cpu_name);
#elif ARCH_PPC
  return print_cpufetch_ppc(cpu, s, cs, term, show_full_cpu_name);
#elif ARCH_ARM
  return print_cpufetch_arm(cpu, s, cs, term);
#elif ARCH_RISCV
  return print_cpufetch_riscv(cpu, s, cs, term);
#endif
}
