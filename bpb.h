#ifndef HW3_BPB_H
#define HW3_BPB_H

#include "fat32.h"

extern BPB_struct* info;
extern char* file;

extern int clsize; //in bytes
extern int reserved; //in bytes
extern int fattotal; //in bytes
extern int validEntrySize; //entries per cluster

void initializeBPBInfo();
#endif
