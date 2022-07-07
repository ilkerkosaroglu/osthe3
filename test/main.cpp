#include <bits/stdc++.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "fat.h"
#include "fat32.h"
#include "bpb.h"
#include "parser.h"
#include "stringTokenizer.h"
#include "algo.h"

// using namespace std;

#define pb push_back
#define pv(x) std::cout<<#x<<": ";for(auto k:x){ std::cout<<k<<" "; }std::cout<<std::endl;
#define dbg(x) std::cout<<#x<<": "<<x<<std::endl;

int n,m,ni,mi;
std::string s;
std::string path1;
std::string path2;
char line[1024];

int32_t main(int argc, char** argv){
	if(argc!=2){
		std::cout<<"argc needs to be 2"<<std::endl;
		return 0;
	}
	
    int fd = open(argv[1], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	file = (char*) mmap(NULL, lseek(fd, 0, SEEK_END), PROT_WRITE, MAP_SHARED, fd, 0);
	initializeBPBInfo();
	//go to /
	cd({""});

	parsed_input* p = new parsed_input();
	while(1){
		printDir();
		std::getline(std::cin, s, '\n');
		if(""==s){
			cd({"."});
			continue;
		}
		strcpy (line, s.c_str());

		clean_input(p);
		parse(p, line);

		input_type type = p->type;
		path1 = p->arg1?p->arg1:"";
		path2 = p->arg2?p->arg2:"";
		switch(type){
			case CD:	
				cd(tokenizeStringPath(path1));
				break;
			case LS:
				if(path1=="-l"){
					ls(true, tokenizeStringPath(path2));
				}else{
					ls(false, tokenizeStringPath(path1));
				}
				break;
			case MKDIR:
				mk(tokenizeStringPath(path1), 1);
				break;
			case TOUCH:
				mk(tokenizeStringPath(path1), 2);
				break;
			case CAT:
				cat(tokenizeStringPath(path1));
				break;
			case MV:
				mv(tokenizeStringPath(path1),tokenizeStringPath(path2));
				break;
			case QUIT:
				break;
			default:
				cd({"."});
				break;
		}

		if(type==QUIT){
			break;
		}
	}
	// clean_input(p);
	// delete p;

	return 0;
}