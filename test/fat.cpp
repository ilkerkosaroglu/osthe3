#include "fat.h"
#include "bpb.h"
#include <bits/stdc++.h>

// holds fat table
int* fat;

// end of chain indicator
int eoc;

int rc;
int32_t fatsize;

int findFirstEmpty(){
    for(int i=rc;i<fatsize;i++){
        if((fat[i]&0xfffffff)==0){
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

    long long sz = info[0].extended.FATSize;
    sz*= BPS;

    // find last chain of cluster
    if(start!=0){
        int cur = start;
        while(!isEoc(fat[cur])){
            cur = fat[cur];
        }
        // change last element to point fe
        for(int i=0;i<info[0].NumFATs;i++){
            *(((int*)(((char*)fat)+(sz * i)))+cur) = fe;
        }
    }
    // fill new entry as eoc
    for(int i=0;i<info[0].NumFATs;i++){
        *(((int*)(((char*)fat)+(sz * i)))+fe) = 0x0FFFFFF8;
    }

    return fe;
}

bool isEoc(int x){
    // std::cout<<std::hex<<(eoc&(0x7FFFFF8))<<std::endl;
    // std::cout<<std::hex<<(x&(0x7FFFFF8))<<std::endl;
    return (0xFFFFFF8)==(x&(0xFFFFFF8));
}

void initializeFATInfo(char* startOfFAT){
    fat = (int*)(startOfFAT);
    eoc = fat[1];
}

void* getClusterPtr(int index){
    return (((char*)fat)+fattotal+clsize*(index-rc));
}