#include "Env/Detector.h"

#include "Change/Branch.h"
#include "Change/Git.h"
#include "Change/Add.h"
#include "Change/Commit.h"

#include <stdio.h>
#include <filesystem>

namespace fs = std::filesystem;

int main(void) {
	fs::path path = fs::current_path();
	std::string literalPath(path.string());

	git::env::Detector dec(literalPath);
	git::change::Branch branch(literalPath, dec.getUser());
	git::change::Git gitMk(literalPath);
	git::change::Add add(literalPath);
	git::change::Commit commit(literalPath, dec.getUser());

	if(dec.check()){
		printf("Branch: %s\n", dec.getUser().branch.c_str());
		printf("Commit with the message \"I WANNA SLEEP\" \n");
		commit.commit("I WANNA SLEEP");
		dec.check();
	}
	else{
		printf("Repository Git is not inicializate\nMaking it\n");
		gitMk.makeGit();
	}



	return 0;
}