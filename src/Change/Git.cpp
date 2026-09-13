#include "Change/Git.h"
#include <stdio.h>

#ifdef WIN32
	#include <direct.h>
	#define mkdir(path, mode) _mkdir(path)
#else
	#include <sys/stat.h>
#endif

using namespace git;

namespace {
	const char infoBuffer[] = "# git ls-files --others --exclude-from=.git/info/exclude\n\
# Lines that start with '#' are comments.\n\
# For a project mostly in C, the following would be a good set of\n\
# exclude patterns (uncomment them if you want to use them):\n\
# *.[oa]\n\
# *~\n";

	const char configBuffer[] = "[core]\n\
\trepositoryformatversion = 0\n\
\tfilemode = true\n\
\tbare = false\n\
\tlogallrefupdates = true\n";
}


void change::Git::makeInfo(){
    mkdir((gitPath + "/info/").c_str(), 0777);

    FILE* Finfo = fopen((gitPath + "/info/exclude").c_str(), "w");
    if(!Finfo) return;

    fprintf(Finfo, "%s", infoBuffer);

    fclose(Finfo);
}

void change::Git::makeobjects(){
    mkdir((gitPath + "/objects/").c_str(), 0777);
    mkdir((gitPath + "/objects/info/").c_str(), 0777);
    mkdir((gitPath + "/objects/pack/").c_str(), 0777);
}

void change::Git::makeRefs(){
    mkdir((gitPath + "/refs/").c_str(), 0777);
    mkdir((gitPath + "/refs/heads/").c_str(), 0777);
    mkdir((gitPath + "/refs/tags/").c_str(), 0777);
} 

void change::Git::makeConfig(){
    FILE* Fconfig = fopen((gitPath + "/config").c_str(), "w");
    if(!Fconfig) return;

    fprintf(Fconfig, "%s", configBuffer);

    fclose(Fconfig);
}

void change::Git::makeDescription(){
    FILE* Fdescription = fopen((gitPath + "/description").c_str(), "w");
    if(!Fdescription) return;

    fprintf(Fdescription, "%s\n", "Unnamed repository; edit this file 'description' to name the repository.");

    fclose(Fdescription);
}

void change::Git::makeHEAD(){
    FILE* Fhead = fopen((gitPath + "/HEAD").c_str(), "w");
    if(!Fhead) return;

    fprintf(Fhead, "%s\n", "ref: refs/heads/master");

    fclose(Fhead);
}

void change::Git::makeGit(){
	mkdir(gitPath.c_str(), 0777);

	makeInfo();
	makeobjects();
	makeRefs(); 
	makeConfig();
	makeDescription();
	makeHEAD();
}