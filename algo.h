#ifndef HW3_ALGO_H
#define HW3_ALGO_H
#include <bits/stdc++.h>
#include "fat.h"
#include "bpb.h"
using namespace std;

class LocInfo{
    vector<string> dir;
    int cluster;
};

struct Entry{
    bool isFile;
    string name;
    vector<string> names;
    int cluster;
    vector<FatFileEntry*> data;
};

void cd(vector<string> path);
void ls(bool detailed, vector<string> path);
void mv();
void mk();
void cat();

#endif
