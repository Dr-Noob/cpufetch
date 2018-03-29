#include <stdio.h>

#define BLACK1 "\x1b[30;1m"
#define BLACK2 "30;1m"
#define BLUE "\x1b[36;1m"


#define BACK_WHITE "\x1b[47;"
#define RED "\x1b[31;1m"
#define GREEN "\x1b[32;1m"
#define RESET "\x1b[0m"

int main(int argc, char const *argv[]) {
  printf(BACK_WHITE BLACK2"%s"BLUE"%s\n","Name: ", "Intel Xeon E5          ");
  printf(BLACK1"%s"BLUE"%s\n","N.Cores: ", "16                  ");
  printf(BLACK1"%s"BLUE"%s\n","Frecuency: ", "2.2GHz            ");
  printf(RESET);
}
