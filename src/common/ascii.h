#ifndef __ASCII__
#define __ASCII__

#define NUMBER_OF_LINES   19
#define LINE_SIZE         62

#define AMD_ASCII \
"                                                              \
                                                              \
                                                              \
                                                              \
                                                              \
                                                              \
     @@@@      @@@       @@@   @@@@@@@@       #############   \
    @@@@@@     @@@@@   @@@@@   @@@    @@@      ############   \
   @@@  @@@    @@@@@@@@@@@@@   @@@      @@@     #     #####   \
  @@@@@@@@@@   @@@  @@@  @@@   @@@      @@@   ###     #####   \
 @@@@    @@@@  @@@       @@@   @@@    @@@    ##############   \
 @@@      @@@  @@@       @@@   @@@@@@@@      #######    ###   \
                                                              \
                                                              \
                                                              \
                                                              \
                                                              \
                                                              \
                                                              "

#define INTEL_ASCII \
"                              ################                \
                      ##############################          \
                 ######                          ######       \
             ###                                     ####     \
        ###                                            ####   \
        ###                                             ####  \
     #                    ###                ###        ####  \
   ##   ###   ########    ######   ######    ###        ####  \
  ##    ###   ###    ###  ###    ####  ####  ###        ####  \
 ##     ###   ###    ###  ###    ###    ###  ###       ####   \
###     ###   ###    ###  ###    ##########  ###     #####    \
###     ###   ###    ###  ###    ###         ###  ######      \
###      ##   ###    ###   #####   #######    ##  ####        \
###                                                           \
####                                                          \
 ####                                        #####            \
   #####                                 ########             \
     ###########                 ############                 \
         #############################                        "
                                                             
#define SNAPDRAGON_ASCII \
"                                                              \
                      @@#######                               \
                 @@@@@@###########                            \
            @ @@@@@@@#################                        \
          @@@@@@@@@@####################                      \
         @@@@@@@@@@@@#####################                    \
        @@@@@@@@@@@@@@@####################                   \
        @@@@@@@@@@@@@@@@@###################                  \
        @@@@@@@@@@@@@@@@@@@@################                  \
           @@@@@@@@@@@@@@@@@@@@#############                  \
       @      @@@@@@@@@@@@@@@@@@############                  \
       @@@        @@@@@@@@@@@@@@@###########                  \
        @@@@@@      @@@@@@@@@@@@@##########                   \
         @@@@@@@@@@  @@@@@@@@@@@@########                     \
           @@@@@@@@@  @@@@@@@@@@#######                       \
              @@@@@@ @@@@@@@@@#######                         \
                  @@@@###########                             \
                                                              \
                                                              "
                                                             
#define MEDIATEK_ASCII \
"                                                              \
                                                              \
                                                              \
                                                              \
                                                              \
                                                              \
      ##  ##  #####  #####   #     ###  @@@@@  @@@@@ @    @   \
     ### # # #       #    #  #    #  #    @   @      @   @    \
    # ###  # # ###   #    #  #   #   #    @   @ @@@  @@@@     \
   #  ##   # #       #    #  #  #    #    @   @      @   @    \
  #   #    # ######  #####   # #     #    @   @@@@@@ @    @   \
                                                              \
                                                              \
                                                              \
                                                              \
                                                              \
                                                              \
                                                              \
                                                              "

#define EXYNOS_ASCII \
"                                                              \
                                                              \
                                                              \
                                                              \
                                                              \
                                                              \
                              ##  ##  ##                      \
                                ##  ##                        \
                                  ##                          \
                                ##  ##                        \
                              ##  ##  ##                      \
                                                              \
                      SAMSUNG                                 \
                      Exynos                                  \
                                                              \
                                                              \
                                                              \
                                                              \
                                                              "
                                                              
#define KIRIN_ASCII \
"                                                              \
                                                              \
                                                              \
                                                              \
                                                              \
      ##                             ####                     \
       #######             #################                  \
          ######################################              \
            #######################################           \
               #######################################        \
                  #############################               \
                ##########################                    \
             #########################                        \
           ########################                           \
        ########################                              \
     #########################                                \
   #########################                                  \
                                                              \
                                                              "                                                                                                                           
                                                              
#define BROADCOM_ASCII \
"                                                              \
                    ################                          \
               #########################                      \
            ###############################                   \
          ################@@@@################                \
         ################@@@@@@################               \
        #################@@@@@@################               \
       #################@@@@@@@@#################             \
       #################@@@@@@@@#################             \
      #################@@@@##@@@@################             \
       ################@@@@##@@@@################             \
       ###############@@@@####@@@@###############             \
        ###@@@@@@@####@@@@####@@@@####@@@@@@@###              \
         @@#######@@@@@@@######@@@@@@@#######@@               \
           ##################################                 \
             ##############################                   \
                ########################                      \
                    ###############                           \
                                                              "
                                                                                                              
#define ARM_ASCII \
"                                                              \
                                                              \
                                                              \
                                                              \
                                                              \
                                                              \
       #####  ####   ####  ######   ####  ######  #######     \
    ##############   ###########    #######################   \
  ####       #####   #####          #####    ######    #####  \
 ####         ####   ####           ####      ####      ####  \
 ####         ####   ####           ####      ####      ####  \
  ####       #####   ####           ####      ####      ####  \
    ##############   ####           ####      ####      ####  \
      ######  ####   ####           ####      ####      ####  \
                                                              \
                                                              \
                                                              \
                                                              \
                                                              "                                                                                                               

#define UNKNOWN_ASCII \
"                                                              \
                                                              \
                                                              \
                        #####                                 \
                    #############                             \
                   ####       ####                            \
                              ####                            \
                             ####                             \
                           ####                               \
                         ####                                 \
                        ####                                  \
                        ####                                  \
                                                              \
                        ####                                  \
                        ####                                  \
                                                              \
                                                              \
                                                              \
                                                              "
                                                             
static const char* ASCII_ARRAY [] = {
  AMD_ASCII,
  INTEL_ASCII,
  ARM_ASCII,
  SNAPDRAGON_ASCII,
  MEDIATEK_ASCII,
  EXYNOS_ASCII,
  KIRIN_ASCII,
  BROADCOM_ASCII,
  UNKNOWN_ASCII
};                                                             

#endif
