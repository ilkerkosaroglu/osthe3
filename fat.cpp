#include "fat.h"
#include "bpb.h"

// holds fat table
int* fat;

// end of chain indicator
int eoc;

int rc;
int32_t fatsize;

int findFirstEmpty(){
    for(int i=rc;i<fatsize;i++){
        if(fat[i]==0){
            return i;
        }
    }
    return -1;
}

int addChain(int start){
    int fe = findFirstEmpty();
    if(fe==-1){
        return -1;
    }

    // find last chain of cluster
    if(start!=0){
        int cur = start;
        while(!isEoc(fat[cur])){
            cur = fat[cur];
        }
        // change last element to point fe
        for(int i=0;i<info[0].NumFATs;i++){
            *(((int*)((char*)fat+(info[0].extended.FATSize * BPS * i)))+cur) = fe;
        }
    }
    // fill new entry as eoc
    for(int i=0;i<info[0].NumFATs;i++){
        *(((int*)((char*)fat+(info[0].extended.FATSize * BPS * i)))+fe) = eoc;
    }

    return fe;
}

bool isEoc(int x){
    return (eoc&(0xFFFFFF8))==(x&(0xFFFFFF8));
}

void initializeFATInfo(char* startOfFAT){
    fat = (int*)(startOfFAT);
    eoc = fat[1];
}

void* getClusterPtr(int index){
    return (((char*)fat)+fattotal+clsize*(index-rc));
}