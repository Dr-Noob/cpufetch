#ifndef __ASCII__
#define __ASCII__

#define COLOR_NONE         ""
#define COLOR_FG_BLACK     "\x1b[30;1m"
#define COLOR_FG_RED       "\x1b[31;1m"
#define COLOR_FG_GREEN     "\x1b[32;1m"
#define COLOR_FG_YELLOW    "\x1b[33;1m"
#define COLOR_FG_BLUE      "\x1b[34;1m"
#define COLOR_FG_MAGENTA   "\x1b[35;1m"
#define COLOR_FG_CYAN      "\x1b[36;1m"
#define COLOR_FG_WHITE     "\x1b[37;1m"
#define COLOR_BG_BLACK     "\x1b[40;1m"
#define COLOR_BG_RED       "\x1b[41;1m"
#define COLOR_BG_GREEN     "\x1b[42;1m"
#define COLOR_BG_YELLOW    "\x1b[43;1m"
#define COLOR_BG_BLUE      "\x1b[44;1m"
#define COLOR_BG_MAGENTA   "\x1b[45;1m"
#define COLOR_BG_CYAN      "\x1b[46;1m"
#define COLOR_BG_WHITE     "\x1b[47;1m"
#define COLOR_FG_B_BLACK   "\x1b[90;1m"
#define COLOR_FG_B_RED     "\x1b[91;1m"
#define COLOR_FG_B_GREEN   "\x1b[92;1m"
#define COLOR_FG_B_YELLOW  "\x1b[93;1m"
#define COLOR_FG_B_BLUE    "\x1b[94;1m"
#define COLOR_FG_B_MAGENTA "\x1b[95;1m"
#define COLOR_FG_B_CYAN    "\x1b[96;1m"
#define COLOR_FG_B_WHITE   "\x1b[97;1m"
#define COLOR_RESET        "\x1b[m"

struct ascii_logo {
  char* art;
  uint32_t width;
  uint32_t height;
  bool replace_blocks;
  char color_ascii[8][100];
  char color_text[2][100];
};

/*
 * ASCII logos brief documentation
 * ----------------------------------------------------
 * C1, C2, ...: ColorN, gets replaced by printer.c with
 *              the color in ascii_logo->color_ascii[N]
 * CR:          Color reset, gets replaced by the reset
 *              color by printer.c
 *
 * Logos with replace_blocks=true are replaced by character
 * blocks (actually, spaces with background color), so
 * the color in the structure must be COLOR_BG_XXX. When
 * replace_blocks is true, the characters '#' are replaced
 * by spaces printed with color_ascii[0], and '@' are
 * printed with color_ascii[1]. If replace_blocks=true,
 * color format specified in ASCIIs ($C1, $C2) are ignored.
 *
 * In any case, '$' is a illegal character to be used in
 * the ascii logos because it is used to parse colors
 *
 * LONG_LOGOS will be printed only if the fit in the screen,
 * otherwise SHORT_LOGOS will be used
 */

// SHORT LOGOS //
#define ASCII_AMD \
"$C2          '###############             \
$C2             ,#############            \
$C2                      .####            \
$C2              #.      .####            \
$C2            :##.      .####            \
$C2           :###.      .####            \
$C2           #########.   :##            \
$C2           #######.       ;            \
$C1                                       \
$C1    ###     ###      ###   #######     \
$C1   ## ##    #####  #####   ##     ##   \
$C1  ##   ##   ### #### ###   ##      ##  \
$C1 #########  ###  ##  ###   ##      ##  \
$C1##       ## ###      ###   ##     ##   \
$C1##       ## ###      ###   #######     "

#define ASCII_INTEL \
"$C1                   .#################.          \
$C1              .####                   ####.     \
$C1          .##                             ###   \
$C1       ##                          :##     ###  \
$C1    #                ##            :##      ##  \
$C1  ##   ##  ######.   ####  ######  :##      ##  \
$C1 ##    ##  ##:  ##:  ##   ##   ### :##     ###  \
$C1##     ##  ##:  ##:  ##  :######## :##    ##    \
$C1##     ##  ##:  ##:  ##   ##.   .  :## ####     \
$C1##      #  ##:  ##:  ####  #####:   ##          \
$C1 ##                                             \
$C1  ###.                         ..o####.         \
$C1   ######oo...         ..oo#######              \
$C1          o###############o                     "

#define ASCII_SNAPD \
"              $C1@@$C2########               \
           $C1@@@@@$C2###########            \
     $C1@@  @@@@@$C2#################        \
   $C1@@@@@@@@@@$C2####################      \
  $C1@@@@@@@@@@@@$C2#####################    \
 $C1@@@@@@@@@@@@@@@$C2####################   \
 $C1@@@@@@@@@@@@@@@@@$C2###################  \
 $C1@@@@@@@@@@@@@@@@@@@@$C2################  \
    $C1@@@@@@@@@@@@@@@@@@@@$C2#############  \
       $C1@@@@@@@@@@@@@@@@@@$C2############  \
$C1@          @@@@@@@@@@@@@@@$C2###########  \
 $C1@@@@@       @@@@@@@@@@@@@$C2##########   \
  $C1@@@@@@@@@   @@@@@@@@@@@@$C2########     \
    $C1@@@@@@@@@  @@@@@@@@@@$C2#######       \
       $C1@@@@@@@@@@@@@@@@$C2#######         \
           $C1@@@@$C2###########             "

#define ASCII_MTK \
"$C1    ##  ## ######  ######  #     ### $C2@@@@@@ @@@@@@ @@  @@  \
$C1   ### ### #       #    #  #    ####   $C2@@   @      @@ @@   \
$C1  ######## # ###   #    #  #   ## ##   $C2@@   @ @@@  @@@@    \
$C1 ## ### ## #       #    #  #  ##  ##   $C2@@   @      @@ @@   \
$C1##  ##  ## ######  #####   # ##   ##   $C2@@   @@@@@@ @@  @@  "

#define ASCII_EXYNOS \
"$C2                      \
$C2                      \
$C2                      \
     $C1##$CR  $C1##$CR  $C1##$CR       \
       $C1##$CR  $C1##$CR         \
         $C1##$CR           \
       $C1##$CR  $C1##$CR         \
     $C1##$CR  $C1##$CR  $C1##$CR       \
$C2                      \
$C2     SAMSUNG          \
$C2     Exynos           \
$C2                      \
$C2                      "

#define ASCII_KIRIN \
"$C1                                #######              \
$C1    #####             ####################           \
$C1       ######################################        \
$C1         #######################################     \
$C1            #######################################  \
$C1              ##############################         \
$C1             ##########################              \
$C1          #########################                  \
$C1        ########################                     \
$C1     ########################                        \
$C1  #########################                          \
$C1#########################                            "

#define ASCII_BROADCOM \
"$C2                                            \
$C2             ################               \
$C2        ##########################          \
$C2     ################################       \
$C2   ################$C1@@@@$C2################     \
$C2  ################$C1@@@@@@$C2################    \
$C2 #################$C1@@@@@@$C2#################   \
$C2#################$C1@@@@@@@@$C2#################  \
$C2#################$C1@@@@@@@@$C2#################  \
$C2################$C1@@@@$C2##$C1@@@@$C2################  \
$C2################$C1@@@@$C2##$C1@@@@$C2################  \
$C2###############$C1@@@@$C2####$C1@@@@$C2###############  \
$C1 @@@@@@@@@@$C2####$C1@@@@$C2####$C1@@@@$C2####$C1@@@@@@@@@@   \
$C2  ######$C1@@@@@@@@@@$C2######$C1@@@@@@@@@@$C2######    \
$C2    ##################################      \
$C2      ##############################        \
$C2         ########################           \
$C2             ###############                \
$C2                                            "

#define ASCII_ARM \
"$C1   #####  ##   # #####  ## ####  ######   \
$C1 ###    ####   ###      ####  ###   ###   \
$C1###       ##   ###      ###    ##    ###  \
$C1 ###    ####   ###      ###    ##    ###  \
$C1  ######  ##   ###      ###    ##    ###  "

// jp2a --height=17 ibm.jpg
#define ASCII_IBM \
"$C1######### ################   #########        #########  \
$C1                                                         \
$C1######### ##################  ##########    ##########   \
$C1                                                         \
$C1  #####      ######    ######   ####################     \
$C1                                                         \
$C1  #####      ##############     ####################     \
$C1                                                         \
$C1  #####      ######    ######   #####  ######  #####     \
$C1                                                         \
$C1######### ################## ########   ####   ########  \
$C1                                                         \
$C1######### ################   ########    ##    ########  "

// inspired by the neofetch mac logo
#define ASCII_APPLE \
"$C1                    'c.         \
$C1                 ,xNMM.         \
$C1               .OMMMMo          \
$C1               OMMM0,           \
$C1     .;loddo:' loolloddol;.     \
$C1   cKMMMMMMMMMMNWMMMMMMMMMM0:   \
$C1 .KMMMMMMMMMMMMMMMMMMMMMMMWd.   \
$C1 XMMMMMMMMMMMMMMMMMMMMMMMX.     \
$C1;MMMMMMMMMMMMMMMMMMMMMMMM:      \
$C1:MMMMMMMMMMMMMMMMMMMMMMMM:      \
$C1.MMMMMMMMMMMMMMMMMMMMMMMMX.     \
$C1 kMMMMMMMMMMMMMMMMMMMMMMMMWd.   \
$C1 .XMMMMMMMMMMMMMMMMMMMMMMMMMMk  \
$C1  .XMMMMMMMMMMMMMMMMMMMMMMMMK.  \
$C1    kMMMMMMMMMMMMMMMMMMMMMMd    \
$C1     ;KMMMMMMMWXXWMMMMMMMk.     \
$C1       .cooc,.    .,coo:.       "

// --------------------- LONG LOGOS ------------------------- //
#define ASCII_AMD_L \
"$C1                                                              \
$C1                                                              \
$C1                                                              \
$C1                                                              \
$C1                                                              \
$C1                                                              \
$C1     @@@@      @@@       @@@   @@@@@@@@      $C2  ############   \
$C1    @@@@@@     @@@@@   @@@@@   @@@    @@@    $C2    ##########   \
$C1   @@@  @@@    @@@@@@@@@@@@@   @@@      @@   $C2   #     #####   \
$C1  @@@    @@@   @@@  @@@  @@@   @@@      @@   $C2 ###     #####   \
$C1 @@@@@@@@@@@@  @@@       @@@   @@@    @@@    $C2#########  ###   \
$C1 @@@      @@@  @@@       @@@   @@@@@@@@@     $C2########    ##   \
$C1                                                              \
$C1                                                              \
$C1                                                              \
$C1                                                              \
$C1                                                              \
$C1                                                              \
$C1                                                              "

#define ASCII_INTEL_L \
"$C1                               ###############@               \
$C1                       ######@                ######@         \
$C1                  ###@                              ###@      \
$C1              ##@                                     ###@    \
$C1         ##@                                             ##@  \
$C1         ##@                                             ##@  \
$C1      @                    ##@                ##@        ##@  \
$C1    #@   ##@   ########@   #####@   #####@    ##@        ##@  \
$C1   #@    ##@   ##@    ##@  ##@    ###@  ###@  ##@        ##@  \
$C1  #@     ##@   ##@    ##@  ##@    ##@    ##@  ##@       ##@   \
$C1 #@      ##@   ##@    ##@  ##@    #########@  ##@     ###@    \
$C1 #@      ##@   ##@    ##@  ##@    ##@         ##@   ####@     \
$C1 #@       #@   ##@    ##@   ####@  ########@   #@  ##@        \
$C1 ##@                                                          \
$C1  ##@                                                         \
$C1  ###@                                        ###@            \
$C1    ####@                               #########@            \
$C1      #########@               ###############@               \
$C1          ##############################@                     "

#define ASCII_ARM_L \
"$C1     ############     ##########   ####  ######  ########   \
$C1  ###############     #########    #######################  \
$C1 ####        ####     ####         #####   #######   #####  \
$C1####         ####     ####         ####     #####     ####  \
$C1####         ####     ####         ####     ####      ####  \
$C1 ####       #####     ####         ####     ####      ####  \
$C1  ###############     ####         ####     ####      ####  \
$C1   ########  ####     ####         ####     ####      ####  "

#define ASCII_IBM_L \
"$C1 ############ ################   ##########        ########## \
$C1                                                              \
$C1 ############ ################## ############    ############ \
$C1                                                              \
$C1    ######       ######    ######    ####################     \
$C1                                                              \
$C1    ######       ##############      ####################     \
$C1                                                              \
$C1    ######       ######    ######    #####  ######  #####     \
$C1                                                              \
$C1 ############ ################## #########   ####   ######### \
$C1                                                              \
$C1 ############ ################   #########    ##    ######### "

typedef struct ascii_logo asciiL;

//                     ------------------------------------------------------------------------------------------------------
//                     | LOGO          | W | H | REPLACE | COLORS LOGO (>0 && <10)        | COLORS TEXT (=2)                |
//                     ------------------------------------------------------------------------------------------------------
asciiL logo_amd      = { ASCII_AMD,      39, 15, false, {COLOR_FG_WHITE, COLOR_FG_GREEN}, {COLOR_FG_WHITE,   COLOR_FG_GREEN}    };
asciiL logo_intel    = { ASCII_INTEL,    48, 14, false, {COLOR_FG_CYAN},                  {COLOR_FG_CYAN,    COLOR_FG_WHITE}    };
asciiL logo_snapd    = { ASCII_SNAPD,    39, 16, false, {COLOR_FG_RED, COLOR_FG_WHITE},   {COLOR_FG_RED,     COLOR_FG_WHITE}    };
asciiL logo_mtk      = { ASCII_MTK,      59,  5, false, {COLOR_FG_BLUE, COLOR_FG_YELLOW}, {COLOR_FG_BLUE,    COLOR_FG_YELLOW}   };
asciiL logo_exynos   = { ASCII_EXYNOS,   22, 13, true,  {COLOR_BG_BLUE, COLOR_FG_WHITE},  {COLOR_FG_BLUE,    COLOR_FG_WHITE}    };
asciiL logo_kirin    = { ASCII_KIRIN,    53, 12, false, {COLOR_FG_RED},                   {COLOR_FG_WHITE,   COLOR_FG_RED}      };
asciiL logo_broadcom = { ASCII_BROADCOM, 44, 19, false, {COLOR_FG_WHITE, COLOR_FG_RED},   {COLOR_FG_WHITE,   COLOR_FG_RED}      };
asciiL logo_arm      = { ASCII_ARM,      42,  5, false, {COLOR_FG_CYAN},                  {COLOR_FG_WHITE,   COLOR_FG_CYAN}     };
asciiL logo_ibm      = { ASCII_IBM,      57, 13, true,  {COLOR_BG_CYAN, COLOR_FG_WHITE},  {COLOR_FG_CYAN,    COLOR_FG_WHITE}    };
asciiL logo_apple    = { ASCII_APPLE,    32, 17, false, {COLOR_FG_WHITE},                 {COLOR_FG_B_BLACK, COLOR_FG_B_WHITE}  };
// Long variants       | ----------------------------------------------------------------------------------------------------|
asciiL logo_amd_l    = { ASCII_AMD_L,    62, 19, true,  {COLOR_BG_WHITE, COLOR_BG_GREEN}, {COLOR_FG_WHITE, COLOR_FG_GREEN}  };
asciiL logo_intel_l  = { ASCII_INTEL_L,  62, 19, true,  {COLOR_BG_CYAN, COLOR_BG_WHITE},  {COLOR_FG_CYAN,  COLOR_FG_WHITE}  };
asciiL logo_arm_l    = { ASCII_ARM_L,    60,  8, true,  {COLOR_BG_CYAN},                  {COLOR_FG_WHITE, COLOR_FG_CYAN}   };
asciiL logo_ibm_l    = { ASCII_IBM_L,    62, 13, true,  {COLOR_BG_CYAN, COLOR_FG_WHITE},  {COLOR_FG_CYAN,  COLOR_FG_WHITE}  };
asciiL logo_unknown  = { NULL,            0,  0, false, {COLOR_NONE},                     {COLOR_NONE,     COLOR_NONE}      };

#endif
