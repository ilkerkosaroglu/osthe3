#ifndef HW3_FAT_H
#define HW3_FAT_H
#include "fat32.h"

// holds fat table
extern int* fat;

// holds cluster table
extern int* cl;

// end of chain indicator
extern int eoc;

extern int rc;
extern int32_t fatsize;

// adds a cluster to the chain
int addChain(int start);

void initializeFATInfo(char* startOfFAT);
void* getClusterPtr(int index);


#endif 