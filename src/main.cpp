#include "Env/Detector.h"
#include "Change/Branch.h"

#include <stdio.h>
#include <filesystem>

namespace fs = std::filesystem;

int main(void) {
	fs::path path = fs::current_path();
	std::string literalPath(path.string());

	git::env::Detector dec(literalPath);
	git::change::Branch branch(literalPath, dec.getUser());


	git::envState state = dec.check();
	
	if(state == git::all_right){
		printf("Rama: %s", dec.getUser().branch.c_str());
		printf("Creando rama 'hola'\n");
		branch.createNewBranch("hola");
		printf("Cambiando a la rama 'hola'\n");
		branch.changeBranch("hola");
	}
	else{
		printf("Sin rama actual\n");
	}



	return 0;
}