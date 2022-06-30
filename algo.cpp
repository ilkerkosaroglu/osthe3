#include "algo.h"
#include "fat.h"
#include "bpb.h"
#include <bits/stdc++.h>

using namespace std;
vector<string> wd;
int wcluster;
int pcluster;
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
            if(folder&&!(entry->msdos.attributes&0x10))return -1; //if item is file & we expected a folder
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
                if(c==32)break;
                s+=c;
            }
            if(ffe[i].msdos.extension[0]!=32)
            s+='.';
            for(int j=0;j<3;j++){
                char c = (char)ffe[i].msdos.extension[j];
                if(c==32)break;
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
        cout<<constructName(v[i])<<(i==v.size()-1?"\n":" ");
    }
    printDir();
}