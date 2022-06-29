#include <bits/stdc++.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "fat32.h"

int main(int argc, char** argv){
    if (argc<=1){
        std::cerr<<"NO IMAGE FILE "<<std::endl;
        return 0; 
    } 
    char* image_path = argv[1];

    
    int file_read  = open(image_path, O_RDONLY, 0);
    
    int file_write = open(image_path, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

    off_t fsize;
    fsize = lseek(file_read, 0, SEEK_END);

    char* common = reinterpret_cast<char*>( mmap(NULL, fsize, PROT_WRITE, 
    MAP_SHARED, file_write, 0));

    struct_BPB_struct* buffer = reinterpret_cast<struct_BPB_struct*>( mmap(NULL, fsize, PROT_WRITE, MAP_SHARED, file_write, 0));
    lseek(file_write, 0, 0);
    int jump_location = 0 ;
    jump_location += buffer[0].BytesPerSector * buffer[0].ReservedSectorCount ;//starting point of fat
    jump_location += buffer[0].NumFATs * buffer[0].extended.FATSize * buffer[0].BytesPerSector ;

    FatFileEntry* data = reinterpret_cast<FatFileEntry*>( common+jump_location);

    std::cout<<(char)data[2].lfn.name1[0]<<std::endl;
    std::cout<<(char)data[2].lfn.name1[1]<<std::endl;
    std::cout<<(char)data[2].lfn.name1[2]<<std::endl;
    std::cout<<(char)data[2].lfn.name1[3]<<std::endl;
    std::cout<<(char)data[2].lfn.name1[4]<<std::endl;

    data[2].lfn.name1[0] = (uint16_t) 't';

    std::cout<<(int)buffer[0].BS_JumpBoot[0]<<std::endl;
    std::cout<<(uint32_t)buffer[0].extended.RootCluster<<std::endl;
    //uint32_t root = reserved bytes + fat kismi ;
    //     root + 1 ;
    return 0;

}