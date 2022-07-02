#include "algo.h"
#include "fat.h"
#include "bpb.h"
#include <bits/stdc++.h>

using namespace std;
vector<string> wd;
int wcluster;
int pcluster;
#define pv(x) std::cout<<#x<<": ";for(auto k:x){ std::cout<<k<<" "; }std::cout<<std::endl;
#define dbg(x) std::cout<<#x<<": "<<x<<std::endl;

void printDir(){
    if(wd.size()==1){
        cout<<"/>";
        return;
    }
    for(int i=0;i<wd.size();i++){
        cout<<(wd[i]);
        cout<<(i==wd.size()-1?">":"/");
    }
}

int findItemCluster(int c, string s, int folder=0){
    auto list = listFiles(c);

    for(auto k:list){
        if(constructName(k) == s){
            auto entry = k.data[k.data.size()-1];
            if((folder==1)&&!(entry->msdos.attributes&0x10))return -1; //if item is file & we expected a folder
            if((folder==2)&&(entry->msdos.attributes&0x10))return -1; //if item is folder & we expected a file
            return entry->msdos.firstCluster;
        }
    }
    return -1;
}

LocInfo locate(vector<string> path){
    LocInfo info;
    vector<string> abspath = wd;
    int wc = wcluster;
    int pc = pcluster;
    for(auto k:path){
        if(k==""){
            abspath.clear();
            abspath.push_back("");
            wc = rc;
            pc = rc;
            continue;
        }
        if(k=="."){
            continue;
        }
        if(k==".."){
            if(abspath.size()>1){
                int clusterOfItem = findItemCluster(wc, "..", 1);
                if(clusterOfItem==-1){
                    info.error = 1;
                    return info;
                }
                if(clusterOfItem==0)clusterOfItem=rc;
                abspath.pop_back();
                wc = pc;
                pc = clusterOfItem;
            }
            continue;
        }

        int clusterOfItem = findItemCluster(wc, k, 1);
        if(clusterOfItem==-1){
            info.error = 1;
            return info;
        }
        abspath.push_back(k);
        pc = wc;
        wc = clusterOfItem;
    }
    info.cluster = wc;
    info.pcluster = pc;
    info.dir = abspath;
    return info;
}

string constructName(Entry e){
    string s;
    s+=e.name;
    // cerr<<e.names.size()<<endl;
    if(e.names.size()!=0){
        s="";
        for(auto it=e.names.rbegin();it!=e.names.rend();it++){
            s+=*it;
        }
    }
    return s;
}

vector<Entry> listFiles(int dircluster){
    vector<Entry> v;
    string s;
    bool lastEntryWas83 = true;
    int curCluster = dircluster;
    do{
        // dbg(curCluster);
        // dbg(eoc);
    FatFileEntry* ffe = (FatFileEntry*)getClusterPtr(curCluster);
    for(int i=0;i<validEntrySize;i++){
        s = "";
        if(ffe[i].msdos.filename[0]==0 || ffe[i].msdos.filename[0]==0xE5){
            continue;
        }
        if(lastEntryWas83){
            v.push_back({});
        }
        //lfn
        if(ffe[i].msdos.attributes == 0x0f){
            lastEntryWas83 = false;
            char c;

            for(int j=0;j<13;j++){
                if(j<5){
                    c = (char)ffe[i].lfn.name1[j];
                }else if(j<11){
                    c = (char)ffe[i].lfn.name2[j-5];
                }else{
                    c = (char)ffe[i].lfn.name3[j-11];
                }
                if(c==-1)continue;
                if(c!='\0')
                s+=c;
            }
            v[v.size()-1].names.push_back(s);
        }else{
            lastEntryWas83=true;
            for(int j=0;j<8;j++){
                char c = (char)ffe[i].msdos.filename[j];
                v[v.size()-1].filename += c;
                if(c==32)continue;
                s+=c;
            }
            if(ffe[i].msdos.extension[0]!=32)
            s+='.';
            for(int j=0;j<3;j++){
                char c = (char)ffe[i].msdos.extension[j];
                v[v.size()-1].extension += c;
                if(c==32)continue;
                s+=c;
            }
            v[v.size()-1].name = s;
            int clus = (ffe[i].msdos.eaIndex << 2) + ffe[i].msdos.firstCluster;

        }

        v[v.size()-1].data.push_back(ffe+i);
    }
    }while(!isEoc(curCluster=fat[curCluster]));
    return v;
}

unsigned char calcChecksum(FatFileEntry* f){
    unsigned char ans = 0;
    for(int i=8;i<8;i++){
        ans = ((ans & 1) << 7) + (ans >> 1) + f->msdos.filename[i];
    }
    for(int i=0;i<3;i++){
        ans = ((ans & 1) << 7) + (ans >> 1) + f->msdos.extension[i];
    }
    return ans;
}

FilePathInfo locateFilePath(vector<string> path, int folder){
    FilePathInfo fpinfo;
    if(path.size()==0){
        fpinfo.error = 1;
        return fpinfo;
    }

    vector<string> dir = path;
    string filename = dir.back();
    dir.pop_back();
    auto info = locate(dir); 

    if(info.error){
        fpinfo.error = 1;
        return fpinfo;
    }

    // int itemc = findItemCluster(info.cluster, filename, folder);
    // if(itemc==-1){
    //     fpinfo.error = 1;
    //     return fpinfo;
    // }

    fpinfo.locInfoDir = info;
    // fpinfo.cluster = itemc;
    return fpinfo;
}

vector<FatFileEntry*> allocateEntries(int dircluster, int num){
    vector<FatFileEntry*> v;
    int curCluster = dircluster;
    do{
        // dbg(curCluster);
        // dbg(eoc);
    FatFileEntry* ffe = (FatFileEntry*)getClusterPtr(curCluster);
    for(int i=0;i<validEntrySize;i++){
        if(ffe[i].msdos.filename[0]==0 || ffe[i].msdos.filename[0]==0xE5){
            v.push_back(ffe+i);
            if(v.size()==num)return v;
        }
    }
    }while(!isEoc(curCluster=fat[curCluster]));
    //try again
    if(v.size()<num){
        addChain(dircluster);
        return allocateEntries(dircluster, num); 
    }
    return v;
}

string findUnique(int dircluster, string extension, int num){
    auto name = "~"+to_string(num);
    while(name.size()<8){
        name+=" ";
    }
    name+=extension;
    while(name.size()<11){
        name+=" ";
    }

    auto list = listFiles(dircluster);
    for(auto k:list){
        if(strcmp(name.substr(0,8).c_str(), k.filename.c_str())==0 && strcmp(name.substr(9,3).c_str(), k.extension.c_str())==0){
            //try again
            return findUnique(dircluster, extension, num+1);
        }
    }
    return name;
}

bool isUnique(int dircluster, string name){
    dbg("ok");
    auto list = listFiles(dircluster);
    for(auto k:list){
        if(constructName(k)==name){
            return false;
        }
    }
    return true;
}

//folder:1, file:2
void mk(vector<string> path, int folder){
    int attr = 0x00;
    if(folder==1)attr=0x10;

    FilePathInfo fpinfo = locateFilePath(path, folder);
    if(fpinfo.error==1){
        printDir();
        return;
    }

    string filename = path.back();
    if(filename.size()==0){
        printDir();
        return;
    }

    if(filename[0]=='.'){
        printDir();
        return;
    }
    dbg(fpinfo.locInfoDir.cluster);
    if(!isUnique(fpinfo.locInfoDir.cluster,filename)){
        printDir();
        return;
    }

    string extension = "   ";
    int started = 0;
    for(int i=0;i<filename.size();i++){
        if(started == 4){
            printDir();
            return;
        }
        if(filename[i]=='.'){
            started = 1;
        }
        if(started){
            extension[started-1] = filename[i];
            started++;
        }
    }

    string msname = findUnique(fpinfo.locInfoDir.cluster, extension, 1);
    vector<string> namesList;
    for(int i=0;i<filename.size();i+=13){
        namesList.push_back(filename.substr(i,13));
    }
    auto list = allocateEntries(fpinfo.locInfoDir.cluster, namesList.size()+1);
    for(int i=0;i<8;i++){
        uint16_t c = msname[i];
        if(i==0&&c==0xe5)c=0x05;
        list[0]->msdos.filename[i] = c;
    }
    for(int i=0;i<3;i++){
        uint16_t c = msname[8+i];
        list[0]->msdos.extension[i] = c;
    }
    auto chk = calcChecksum(list[0]);
    list[0]->msdos.attributes = attr;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    list[0]->msdos.creationTimeMs = t; //? test
    dbg(t);
    dbg(list[0]->msdos.creationTimeMs);
    
    list[0]->msdos.creationTime=0;
    for(int i=0;i<5;i++){
        int sec = tm.tm_sec/2;
        list[0]->msdos.creationTime|=(1<<i) & sec;
    }
    for(int i=0;i<6;i++){
        int min = tm.tm_min;
        list[0]->msdos.creationTime|=((1<<i) & min)<<5;
    }
    for(int i=0;i<5;i++){
        int hour = tm.tm_hour;
        list[0]->msdos.creationTime|=((1<<i) & hour)<<11;
    }

    list[0]->msdos.creationDate=0;
    for(int i=0;i<5;i++){
        int day = tm.tm_mday;
        list[0]->msdos.creationDate|=(1<<i) & day;
    }
    for(int i=0;i<4;i++){
        int month = tm.tm_mon;
        list[0]->msdos.creationDate|=((1<<i) & month)<<5;
    }
    for(int i=0;i<5;i++){
        int year = tm.tm_year - 1980;
        list[0]->msdos.creationDate|=((1<<i) & year)<<9;
    }



}

void cd(vector<string> path){
    if(path.size()==0){
        printDir();
        return;
    }

    auto info = locate(path);
    if(!info.error){
        wd = info.dir;
        wcluster = info.cluster;
        pcluster = info.pcluster;
    }
    printDir();
}

void ls(bool detailed, vector<string> path){
    auto info = locate(path);
    if(info.error){
        printDir();
        return;
    }
    auto v = listFiles(info.cluster);
    for(int i=0;i<v.size();i++){
        string s = constructName(v[i]);
        if(s.size()==0 || s[0]=='.')continue;
        cout<<s<<(i==v.size()-1?"\n":" ");
    }
    printDir();
}