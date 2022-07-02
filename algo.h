#ifndef HW3_ALGO_H
#define HW3_ALGO_H
#include <bits/stdc++.h>
#include "fat.h"
#include "bpb.h"
using namespace std;

class LocInfo{
    public:
    vector<string> dir;
    int cluster;
    int pcluster;
    int error = 0;
};

class FilePathInfo{
    public:
    LocInfo locInfoDir;
    int cluster;
    int error = 0;
};

struct Entry{
    bool isFile;
    string name;
    vector<string> names;
    int cluster;
    vector<FatFileEntry*> data;
    string filename;
    string extension;
};

//helpers:
int findItemCluster(int c, string s, int folder);
LocInfo locate(vector<string> path);
vector<Entry> listFiles(int dircluster);
string constructName(Entry e);

void cd(vector<string> path);
void ls(bool detailed, vector<string> path);
void mv();
void mk(vector<string> path, int folder);
void cat();

#endif
