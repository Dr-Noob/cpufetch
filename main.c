#include <stdio.h>
#include <stdlib.h>
#include "02h.h"

int main() {
  struct level2* level2 = fillLevel2(level2);
  debugLevel2(level2);
  freeLevel2(level2);
}
