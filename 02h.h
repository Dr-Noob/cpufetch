#ifndef __02h__
#define __02h__

/***

Cache and TLB's characteristics

***/

struct level2 {
  struct cache* cache;
  struct TLB* tlb;
};

struct level2* fillLevel2(struct level2* level2);
void debugLevel2(struct level2* level2);
void freeLevel2(struct level2* level2);

#endif
