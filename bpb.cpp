#include "bpb.h"
#include "fat.h"

BPB_struct* info;
char* file;

int clsize; //in bytes
int reserved; //in bytes
int fattotal; //in bytes
int validEntrySize; //entries per cluster

void initializeBPBInfo(){
    info = (BPB_struct*)file;
    rc = info[0].extended.RootCluster;
    fatsize = info[0].extended.FATSize * BPS / 4; //because we have 4 bytes per entry
    reserved = info[0].ReservedSectorCount * BPS;
    fattotal = info[0].extended.FATSize * BPS * info[0].NumFATs;
    clsize = BPS * info[0].SectorsPerCluster;
    validEntrySize = clsize/32;
    initializeFATInfo(file+reserved);
}