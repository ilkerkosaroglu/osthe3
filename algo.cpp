#include "algo.h"
#include "fat.h"
#include "bpb.h"
#include <bits/stdc++.h>

using namespace std;
vector<string> wd;
int curcluster;
#define dbg(x) std::cout<<#x<<": "<<x<<std::endl;

// LocInfo locate(vector<string> path){
//     // vector<string>
//     // if(path.size()==0){
//     //     return;
//     // }
//     // if(path[0]==""){
//     //     curcluster = rc;
//     //     wd = 
//     // }

// }

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
    return e.name;
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
            cerr<<"cluster: "<< clus <<endl;

        }

        v[v.size()-1].data.push_back(ffe+i);
    }
    }while((curCluster=fat[curCluster])!=eoc);
    for(int i=0;i<v.size();i++){
        cout<<constructName(v[i])<<(i==v.size()-1?"\n":"|");
    }
    return v;
}

void cd(vector<string> path){
    if(path.size()==0){
        return;
    }
    //if starting from root
    if(path[0]==""){
        wd = path;
    }

    // locate(path);
}

void ls(bool detailed, vector<string> path){
    listFiles(2);
    // if(path.size()==0){
    //     return;
    // }

    // locate(path);
}