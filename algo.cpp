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
        cout<<"/> ";
        return;
    }
    for(int i=0;i<wd.size();i++){
        cout<<(wd[i]);
        cout<<(i==wd.size()-1?"> ":"/");
    }
}

int findItemCluster(int c, string s, int folder=0){
    auto list = listFiles(c);

    for(auto k:list){
        if(constructName(k) == s){
            auto entry = k.data[k.data.size()-1];
            if((folder==1)&&!(entry->msdos.attributes&0x10))return -1; //if item is file & we expected a folder
            if((folder==2)&&(entry->msdos.attributes&0x10))return -1; //if item is folder & we expected a file
            return (((int)entry->msdos.eaIndex<<16) + (int)entry->msdos.firstCluster);
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
                abspath.pop_back();
                wc = pc;
                if(abspath.size()==1){
                    pc = wc;
                    continue;
                }
                int clusterOfItem = findItemCluster(wc, "..", 1);
                if(clusterOfItem==-1){
                    info.error = 1;
                    return info;
                }
                if(clusterOfItem==0)clusterOfItem=rc;
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
            int clus = ((int)ffe[i].msdos.eaIndex << 16) + (int)ffe[i].msdos.firstCluster;

        }

        v[v.size()-1].data.push_back(ffe+i);
    }
    }while(!isEoc(curCluster=fat[curCluster]));
    return v;
}

unsigned char calcChecksum(FatFileEntry* f){
    unsigned char ans = 0;
    for(int i=0;i<8;i++){
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
    FatFileEntry* ffe = (FatFileEntry*)getClusterPtr(curCluster);
    for(int i=0;i<validEntrySize;i++){
        if(ffe[i].msdos.filename[0]==0){
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
        if(strcmp(name.substr(0,8).c_str(), k.filename.c_str())==0 && strcmp(name.substr(8,3).c_str(), k.extension.c_str())==0){
            return findUnique(dircluster, extension, num+1);
        }
    }
    return name;
}

bool isUnique(int dircluster, string name){
    auto list = listFiles(dircluster);
    for(auto k:list){
        if(constructName(k)==name){
            return false;
        }
    }
    return true;
}

//folder:1, file:2
int mk(vector<string> path, int folder, int allocateFolderMem){
    int attr = 0x00;
    if(folder==1)attr=0x10;

    FilePathInfo fpinfo = locateFilePath(path, folder);
    if(fpinfo.error==1){
        return -1;
    }

    string filename = path.back();
    if(filename.size()==0){
        return -1;
    }

    if(filename[0]=='.'){
        return -1;
    }

    if(!isUnique(fpinfo.locInfoDir.cluster,filename)){
        return -1;
    }

    string extension = "   ";
    int started = 0;
    for(int i=0;i<filename.size();i++){
        if(started == 4){
            return -1;
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
    int last = namesList.size();
    for(int i=0;i<8;i++){
        uint16_t c = msname[i];
        if(i==0&&c==0xe5)c=0x05;
        list[last]->msdos.filename[i] = c;
    }
    for(int i=0;i<3;i++){
        uint16_t c = msname[8+i];
        list[last]->msdos.extension[i] = c;
    }
    auto chk = calcChecksum(list[last]);
    list[last]->msdos.attributes = attr;
    
    list[last]->msdos.reserved = 0;

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    
    list[last]->msdos.creationTimeMs = (t%1000) /10; //? test
    
    list[last]->msdos.creationTime=0;
    for(int i=0;i<5;i++){
        int sec = tm.tm_sec/2;
        list[last]->msdos.creationTime|=(1<<i) & sec;
    }
    for(int i=0;i<6;i++){
        int min = tm.tm_min;
        list[last]->msdos.creationTime|=((1<<i) & min)<<5;
    }
    for(int i=0;i<5;i++){
        int hour = tm.tm_hour;
        list[last]->msdos.creationTime|=((1<<i) & hour)<<11;
    }

    list[last]->msdos.creationDate=0;
    for(int i=0;i<5;i++){
        int day = tm.tm_mday;
        list[last]->msdos.creationDate|=(1<<i) & day;
    }
    for(int i=0;i<4;i++){
        int month = tm.tm_mon;
        list[last]->msdos.creationDate|=((1<<i) & month)<<5;
    }
    for(int i=0;i<7;i++){
        int year = tm.tm_year - 1980;
        list[last]->msdos.creationDate|=((1<<i) & year)<<9;
    }

    list[last]->msdos.lastAccessTime = list[last]->msdos.creationDate;

    int newCluster = 0;
    if(folder==1 && allocateFolderMem){
        newCluster = addChain(0);
        if(newCluster==-1) return -1;
    }

    list[last]->msdos.eaIndex = (newCluster >> 16) & 0xffff;
    list[last]->msdos.firstCluster = newCluster & 0xffff;
    
    list[last]->msdos.modifiedDate = list[last]->msdos.creationDate;
    list[last]->msdos.modifiedTime = list[last]->msdos.creationTime;
    list[last]->msdos.fileSize = 0;

    reverse(namesList.begin(),namesList.end());
    for(int i=0;i<namesList.size();i++){
        list[i]->lfn.sequence_number = namesList.size()-i;
        for(int k=0;k<13;k++){
            if(k<5){
                list[i]->lfn.name1[k] = 0;
            }else if(k<11){
                list[i]->lfn.name2[k-5] = 0;
            }else{
                list[i]->lfn.name3[k-11] = 0;
            }
        }
        for(int k=0;k<namesList[i].size();k++){
            if(k<5){
                list[i]->lfn.name1[k] = namesList[i][k];
            }else if(k<11){
                list[i]->lfn.name2[k-5] = namesList[i][k];
            }else{
                list[i]->lfn.name3[k-11] = namesList[i][k];
            }
        }
        list[i]->lfn.attributes = 0x0f;
        list[i]->lfn.reserved = 0x00;
        list[i]->lfn.checksum = chk;
        list[i]->lfn.firstCluster = 0;
    }
    list[0]->lfn.sequence_number |= 0x40;


    if(folder==1){
        FatFileEntry* ffe = (FatFileEntry*)getClusterPtr(newCluster);
        FatFileEntry* parffe = (FatFileEntry*)getClusterPtr(fpinfo.locInfoDir.cluster);
        for(int i=0;i<8;i++){
            ffe[0].msdos.filename[i]=0x20;
            ffe[1].msdos.filename[i]=0x20;
        }
        for(int i=0;i<3;i++){
            ffe[0].msdos.extension[i]=0x20;
            ffe[1].msdos.extension[i]=0x20;
        }
        ffe[0].msdos.filename[0]='.';
        ffe[1].msdos.filename[0]='.';
        ffe[1].msdos.filename[1]='.';
        // ffe[0].msdos
        for(int i=0;i<2;i++){
            auto data = list[last];
            if(i==1){
                data = parffe;
                if(fpinfo.locInfoDir.cluster==rc){
                    data = ffe+2;
                }
            }
            ffe[i].msdos.attributes = data->msdos.attributes;
            ffe[i].msdos.attributes |= 0x10;
            ffe[i].msdos.reserved = data->msdos.reserved;
            ffe[i].msdos.creationTimeMs = data->msdos.creationTimeMs;
            ffe[i].msdos.creationTime = data->msdos.creationTime;
            ffe[i].msdos.creationDate = data->msdos.creationDate;
            ffe[i].msdos.lastAccessTime = data->msdos.lastAccessTime;
            ffe[i].msdos.eaIndex = data->msdos.eaIndex;
            ffe[i].msdos.modifiedTime = data->msdos.modifiedTime;
            ffe[i].msdos.modifiedDate = data->msdos.modifiedDate;
            ffe[i].msdos.firstCluster = data->msdos.firstCluster;
            ffe[i].msdos.fileSize = data->msdos.fileSize;
        }

        //change parent modification date
        if(fpinfo.locInfoDir.cluster!=rc){
            auto entry = parffe+1;
            int parparclu = (((int)entry->msdos.eaIndex<<16) + (int)entry->msdos.firstCluster);
            if(parparclu==0)parparclu=rc;
            FatFileEntry* parparffe = (FatFileEntry*)getClusterPtr(parparclu);
            auto v = listFiles(parparclu);
            for(auto k:v){
                auto kentry = k.data[k.data.size()-1];
                int kclu = (((int)kentry->msdos.eaIndex<<16) + (int)kentry->msdos.firstCluster);
                if(kclu==0)kclu=rc;
                if(kclu == fpinfo.locInfoDir.cluster){
                    kentry->msdos.modifiedDate = list[last]->msdos.modifiedDate;
                    kentry->msdos.modifiedTime = list[last]->msdos.modifiedTime;
                    break;
                }
            }

        }
        
    }

    return 0;
}

void mv(vector<string> path1, vector<string> path2){
    FilePathInfo fpinfo = locateFilePath(path1, 2);
    if(fpinfo.error==1){
        return;
    }

    string filename = path1.back();
    if(filename.size()==0){
        return;
    }

    if(filename[0]=='.'){
        return;
    }
    auto list1 = listFiles(fpinfo.locInfoDir.cluster);
    Entry item;
    int flag = 0;
    for(auto k:list1){
        if(constructName(k)==filename){
            item = k;
            flag = 1;
        }
    }
    if(flag==0)return;

    auto info = locate(path2);
    if(info.error==1){
        return;
    }
    path2.push_back(filename);

    int clc = info.cluster;

    string filename2 = filename;

    if(!isUnique(clc,filename2)){
        return ;
    }
    int type = (item.data[item.data.size()-1]->msdos.attributes&0x10) ? 1:2;
    int res = mk(path2, type, 0);
    if(res==-1){
        return;
    }

    uint16_t eanew;
    uint16_t fcnew;

    if(clc==rc){
        eanew = 0;
        fcnew = 0;
    }else{
        eanew = (clc >> 16) & 0xffff;
        fcnew = (clc) & 0xffff;        
    }

    auto e = item.data[item.data.size()-1];
    

    auto list2 = listFiles(clc);
    for(auto k:list2){
        if(constructName(k)==filename2){
            auto entry = k.data[k.data.size()-1];

            entry->msdos.attributes = e->msdos.attributes;
            entry->msdos.reserved = e->msdos.reserved;
            entry->msdos.creationTimeMs = e->msdos.creationTimeMs;
            entry->msdos.creationTime = e->msdos.creationTime;
            entry->msdos.creationDate = e->msdos.creationDate;
            entry->msdos.lastAccessTime = e->msdos.lastAccessTime;
            entry->msdos.eaIndex = e->msdos.eaIndex;
            entry->msdos.firstCluster = e->msdos.firstCluster;
            entry->msdos.fileSize = e->msdos.fileSize;
            break;
        }
    }

    if(type==1){
        int nextc = (((int)e->msdos.eaIndex<<16) + (int)e->msdos.firstCluster);
        FatFileEntry* pt = (FatFileEntry*)getClusterPtr(nextc);
        pt[1].msdos.eaIndex = eanew;
        pt[1].msdos.firstCluster = fcnew;
    }

    for(int i=0;i<item.data.size();i++){
        item.data[i]->msdos.filename[0] = 0xe5; //mark as deleted
    }

}

void cd(vector<string> path){
    if(path.size()==0){
        return;
    }

    auto info = locate(path);
    if(!info.error){
        wd = info.dir;
        wcluster = info.cluster;
        pcluster = info.pcluster;
    }
}

vector<string> months = {"January","February","March","April","May","June","July","August","October","September","November","December"};

void ls(bool detailed, vector<string> path){
    auto info = locate(path);
    if(info.error){
        return;
    }
    auto v = listFiles(info.cluster);
    if(!detailed){
        for(int i=0;i<v.size();i++){
            string s = constructName(v[i]);
            if(s.size()==0 || s[0]=='.')continue;
            cout<<s<<(i==v.size()-1?"\n":" ");
        }
    }else{
        for(int i=0;i<v.size();i++){
            string s = constructName(v[i]);
            if(s.size()==0 || s[0]=='.')continue;
            if(v[i].data.size()==0)continue;
            FatFileEntry* last = v[i].data[v[i].data.size()-1];
            cout<<((last->msdos.attributes&0x10)?'d':'-')<<"rwx------"<<" ";
            int mon = ((last->msdos.modifiedDate) & 0x1e0)>>5;
            int day = (last->msdos.modifiedDate) & 0x1f;
            int hr = ((last->msdos.modifiedTime) & 0xf800)>>11;
            int mn = ((last->msdos.modifiedTime) & 0x7e0)>>5;
            cout<<1<<" root "<<"root "<<months[mon]<<" "<<std::setfill('0') << std::setw(2)<<day<<std::setw(0)<<" ";
            cout<<std::setw(2)<<hr<<std::setw(0)<<":"<<std::setw(2)<<mn<<" "<<std::setw(0)<<s<<endl;
        }
    }

}

void cat(vector<string> path){
    
    FilePathInfo fpinfo = locateFilePath(path, 2);
    if(fpinfo.error==1){
        return;
    }

    string filename = path.back();
    if(filename.size()==0){
        return;
    }

    if(filename[0]=='.'){
        return;
    }

    int itemc = findItemCluster(fpinfo.locInfoDir.cluster,filename,2);
    if(itemc==-1){
        return;
    }

    int flag = 0;
    do{
        if(itemc==0){
            flag = 1;
            cout<<endl;
            break;
        }
        auto r = (char*)getClusterPtr(itemc);
        for(int i=0;i<clsize;i++){
            if(r[i]=='\0'){
                flag = 1;
                cout<<endl;
                break;
            }
            cout<<r[i];
        }
    }while(!flag && !isEoc(itemc=fat[itemc]));
}